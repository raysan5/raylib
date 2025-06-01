#include "rlvk.h"
#include "utils.h"      // For TRACELOG
#include <stdio.h>      // For TRACELOG / printf
#include <stdlib.h>     // For RL_MALLOC, RL_FREE, NULL
#include <string.h>     // For strcmp, memset
#include <stdbool.h>    // For bool type

// CPU-side vertex buffer
static rlvkVertex *rlvkCPUVertexBuffer = NULL;
static uint32_t rlvkCPUVertexCount = 0;
static uint32_t rlvkCPUVertexBufferCapacity = 0;
static const uint32_t RLVK_DEFAULT_CPU_VERTEX_BUFFER_CAPACITY = RLVK_MAX_BATCH_ELEMENTS * 4; // Same as RL_DEFAULT_BATCH_BUFFER_ELEMENTS * 4

// Current vertex attribute state
static float currentTexcoordX = 0.0f, currentTexcoordY = 0.0f;
static unsigned char currentColorR = 255, currentColorG = 255, currentColorB = 255, currentColorA = 255;

// GPU vertex buffer resources (one per frame in flight / swapchain image)
static rlvkBuffer *rlvkGPUVertexBuffers = NULL;
static VkDeviceSize rlvkGPUVertexBufferSize = 0; // Size of each individual GPU vertex buffer

// Current primitive topology
static int currentPrimitiveMode = 0; // Default to RL_TRIANGLES, will be set by rlvkSetPrimitiveMode

// Default Texture, Sampler, and Descriptor Set
static VkImage vkDefaultTextureImage = VK_NULL_HANDLE;
static VkDeviceMemory vkDefaultTextureImageMemory = VK_NULL_HANDLE;
static VkImageView vkDefaultTextureImageView = VK_NULL_HANDLE;
static VkSampler vkDefaultTextureSampler = VK_NULL_HANDLE;
static VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;
static VkDescriptorSet vkDefaultDescriptorSet = VK_NULL_HANDLE;
// static unsigned int rlvkDefaultTextureId = 0; // Not strictly needed if managed internally

// Shader and Pipeline
static VkShaderModule vkVertShaderModule = VK_NULL_HANDLE;
static VkShaderModule vkFragShaderModule = VK_NULL_HANDLE;
static VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;
static VkPipeline vkGraphicsPipeline = VK_NULL_HANDLE;
static VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;

// Placeholder SPIR-V bytecode (Replace with actual compiled shader data)
// IMPORTANT: These are NOT valid SPIR-V. They are just small placeholders.
// User must compile src/shapes_vert.glsl and src/shapes_frag.glsl to SPIR-V
// (e.g., using glslangValidator) and replace these arrays with the actual bytecode.
static const uint32_t shapes_vert_spv_placeholder[] = {
    0x07230203, 0x00010000, 0x000d000a, 0x0000001b,
    0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    // ... rest of placeholder ...
};
static const uint32_t shapes_frag_spv_placeholder[] = {
    0x07230203, 0x00010000, 0x000d000a, 0x0000000f,
    0x00000000, 0x00020011, 0x00000001, 0x0006000b,
    // ... rest of placeholder ...
};


// Core Vulkan Handles
static VkInstance vkInstance = VK_NULL_HANDLE;
static VkSurfaceKHR vkSurface = VK_NULL_HANDLE;
static VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
static VkDevice vkDevice = VK_NULL_HANDLE;
static VkQueue vkGraphicsQueue = VK_NULL_HANDLE;
static VkQueue vkPresentQueue = VK_NULL_HANDLE;

// Queue Family Indices
typedef struct {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue;
    bool presentFamilyHasValue;
} QueueFamilyIndices;
static QueueFamilyIndices queueFamilyIndices;

// Swapchain related
static VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
static VkFormat vkSwapchainImageFormat;
static VkExtent2D vkSwapchainExtent;
static VkImage* vkSwapchainImages = NULL;
static uint32_t vkSwapchainImageCount = 0;
static VkImageView* vkSwapchainImageViews = NULL;

// Render Pass and Framebuffers
static VkRenderPass vkRenderPass = VK_NULL_HANDLE;
static VkFramebuffer* vkFramebuffers = NULL; // One per swapchain image view

// Depth Buffer
static VkImage vkDepthImage = VK_NULL_HANDLE;
static VkDeviceMemory vkDepthImageMemory = VK_NULL_HANDLE;
static VkImageView vkDepthImageView = VK_NULL_HANDLE;
static VkFormat vkDepthFormat;

// Command Pool and Command Buffers
static VkCommandPool vkCommandPool = VK_NULL_HANDLE;
static VkCommandBuffer* vkCommandBuffers = NULL; // One per framebuffer

// Synchronization Primitives
static VkSemaphore vkImageAvailableSemaphore = VK_NULL_HANDLE;
static VkSemaphore vkRenderFinishedSemaphore = VK_NULL_HANDLE;
static VkFence* vkInFlightFences = NULL; // One per frame in flight (usually same as swapchain image count)
// static VkFence* imagesInFlight; // Maps swapchain images to fences

static bool rlvkReady = false;
static int screenWidth = 0;
static int screenHeight = 0;

// Drawing/Frame state
static uint32_t currentFrame = 0;
#define MAX_FRAMES_IN_FLIGHT 2 // Default to 2, will be set to vkSwapchainImageCount if different
                               // This define might become a static variable if vkSwapchainImageCount can change (e.g. recreation)
static uint32_t acquiredImageIndex = 0; // To store the image index from vkAcquireNextImageKHR

// Clear values for the render pass
static VkClearColorValue currentClearColor = {{0.0f, 0.0f, 0.0f, 1.0f}}; // Default to black
static VkClearDepthStencilValue defaultDepthStencilClear = {1.0f, 0};    // Default depth/stencil clear


// Helper function to find suitable queue families
static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = {0, 0, false, false};
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)RL_MALLOC(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    if (!queueFamilies) {
        TRACELOG(LOG_ERROR, "RLVK: Failed to allocate memory for queue families");
        return indices;
    }
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            indices.graphicsFamilyHasValue = true;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
            indices.presentFamilyHasValue = true;
        }

        if (indices.graphicsFamilyHasValue && indices.presentFamilyHasValue) {
            break;
        }
    }
    RL_FREE(queueFamilies);
    return indices;
}

// Helper function to check device suitability
static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    // TODO: Check for required device extensions (e.g. swapchain)
    // TODO: Query and check surface formats and present modes
    return indices.graphicsFamilyHasValue && indices.presentFamilyHasValue;
}


void rlvkInit(VkInstance instance, VkSurfaceKHR surface, int width, int height) {
    TRACELOG(LOG_INFO, "RLVK: Initializing Vulkan backend.");
    if (rlvkReady) {
        TRACELOG(LOG_WARNING, "RLVK: Vulkan backend already initialized.");
        return;
    }

    vkInstance = instance;
    vkSurface = surface;
    screenWidth = width;
    screenHeight = height;

    if (vkInstance == VK_NULL_HANDLE) {
        TRACELOG(LOG_FATAL, "RLVK: Provided VkInstance is NULL.");
        return;
    }
    if (vkSurface == VK_NULL_HANDLE) {
        TRACELOG(LOG_FATAL, "RLVK: Provided VkSurfaceKHR is NULL.");
        return;
    }

    // --- Physical Device Selection ---
    uint32_t deviceCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(vkInstance, &deviceCount, NULL);
    if (result != VK_SUCCESS || deviceCount == 0) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to find GPUs with Vulkan support!");
        return;
    }

    VkPhysicalDevice* devices = (VkPhysicalDevice*)RL_MALLOC(deviceCount * sizeof(VkPhysicalDevice));
    if (!devices) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to allocate memory for physical devices list.");
        return;
    }
    result = vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices);
    if (result != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to enumerate physical devices.");
        RL_FREE(devices);
        return;
    }

    TRACELOG(LOG_INFO, "RLVK: Found %d physical device(s).", deviceCount);
    VkPhysicalDeviceProperties chosenDeviceProperties; // To store properties of the chosen device for logging or other uses

    for (uint32_t i = 0; i < deviceCount; i++) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
        TRACELOG(LOG_DEBUG, "RLVK: Evaluating device: %s (ID: %u, Type: %u)", deviceProperties.deviceName, deviceProperties.deviceID, deviceProperties.deviceType);

        if (isDeviceSuitable(devices[i], vkSurface)) {
            // Prefer discrete GPU if available
            if (vkPhysicalDevice == VK_NULL_HANDLE || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                vkPhysicalDevice = devices[i];
                chosenDeviceProperties = deviceProperties; // Store its properties
                // If it's a discrete GPU, we might want to break early, or continue to see if there are others.
                // For now, take the first suitable discrete GPU or the first suitable integrated/other if no discrete.
                if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                    TRACELOG(LOG_INFO, "RLVK: Selected discrete GPU: %s", deviceProperties.deviceName);
                    break;
                }
            }
        }
    }
    RL_FREE(devices);

    if (vkPhysicalDevice == VK_NULL_HANDLE) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to find a suitable GPU!");
        return;
    }
    TRACELOG(LOG_INFO, "RLVK: Selected physical device: %s", chosenDeviceProperties.deviceName);


    queueFamilyIndices = findQueueFamilies(vkPhysicalDevice, vkSurface);
    if (!queueFamilyIndices.graphicsFamilyHasValue || !queueFamilyIndices.presentFamilyHasValue) {
        TRACELOG(LOG_FATAL, "RLVK: Could not find required queue families on selected physical device.");
        vkPhysicalDevice = VK_NULL_HANDLE; // Reset since it's not fully suitable
        return;
    }
    TRACELOG(LOG_INFO, "RLVK: Graphics Queue Family Index: %u", queueFamilyIndices.graphicsFamily);
    TRACELOG(LOG_INFO, "RLVK: Present Queue Family Index: %u", queueFamilyIndices.presentFamily);


    // Placeholder for further initialization steps
    // For now, if we reached here with a physical device, consider it partially ready for this phase.
    // rlvkReady = true; // This will be set at the very end of the full Init function.
    TRACELOG(LOG_INFO, "RLVK: Physical device selected successfully. Further initialization pending.");

    // TODO: Implement steps 3-11 as per the plan.
    // For now, stubbing the rest of the function.
    // This is just Phase 1.
    if (vkPhysicalDevice != VK_NULL_HANDLE) {
         // rlvkReady = true; // This will be at the end of the full function
         TRACELOG(LOG_INFO, "RLVK: Stub: Phase 1 (Device Selection) complete.");
    } else {
         TRACELOG(LOG_ERROR, "RLVK: Stub: Phase 1 (Device Selection) failed.");
         rlvkReady = false;
         return;
    }
    // --- Logical Device Creation ---
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfos[2]; // Max 2: one for graphics, one for present (if different)
    uint32_t uniqueQueueFamilyCount = 0;

    // Graphics Queue
    queueCreateInfos[uniqueQueueFamilyCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[uniqueQueueFamilyCount].queueFamilyIndex = queueFamilyIndices.graphicsFamily;
    queueCreateInfos[uniqueQueueFamilyCount].queueCount = 1;
    queueCreateInfos[uniqueQueueFamilyCount].pQueuePriorities = &queuePriority;
    queueCreateInfos[uniqueQueueFamilyCount].pNext = NULL;
    queueCreateInfos[uniqueQueueFamilyCount].flags = 0;
    uniqueQueueFamilyCount++;

    // Present Queue (if different from graphics)
    if (queueFamilyIndices.presentFamily != queueFamilyIndices.graphicsFamily) {
        queueCreateInfos[uniqueQueueFamilyCount].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[uniqueQueueFamilyCount].queueFamilyIndex = queueFamilyIndices.presentFamily;
        queueCreateInfos[uniqueQueueFamilyCount].queueCount = 1;
        queueCreateInfos[uniqueQueueFamilyCount].pQueuePriorities = &queuePriority;
        queueCreateInfos[uniqueQueueFamilyCount].pNext = NULL;
        queueCreateInfos[uniqueQueueFamilyCount].flags = 0;
        uniqueQueueFamilyCount++;
    }

    VkPhysicalDeviceFeatures deviceFeatures = {0}; // Initialize all features to VK_FALSE
    // Enable specific features if needed, e.g. deviceFeatures.samplerAnisotropy = VK_TRUE;

    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkDeviceCreateInfo deviceCreateInfo = {0};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = uniqueQueueFamilyCount;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = sizeof(deviceExtensions) / sizeof(deviceExtensions[0]);
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
    // deviceCreateInfo.enabledLayerCount is deprecated and ignored for vkCreateDevice. Validation layers are instance-level.

    result = vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, NULL, &vkDevice);
    if (result != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create logical device (Error: %i)", result);
        // Potentially reset vkPhysicalDevice here if cleanup is needed
        return;
    }
    TRACELOG(LOG_INFO, "RLVK: Logical device created successfully.");

    vkGetDeviceQueue(vkDevice, queueFamilyIndices.graphicsFamily, 0, &vkGraphicsQueue);
    vkGetDeviceQueue(vkDevice, queueFamilyIndices.presentFamily, 0, &vkPresentQueue);
    TRACELOG(LOG_INFO, "RLVK: Graphics and Present queues obtained.");

    // --- Swapchain Creation ---
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatCount, NULL);
    VkSurfaceFormatKHR* formats = NULL;
    if (formatCount != 0) {
        formats = (VkSurfaceFormatKHR*)RL_MALLOC(formatCount * sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatCount, formats);
    } else {
        TRACELOG(LOG_FATAL, "RLVK: No surface formats found for swapchain creation.");
        vkDestroyDevice(vkDevice, NULL); vkDevice = VK_NULL_HANDLE;
        return;
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModeCount, NULL);
    VkPresentModeKHR* presentModes = NULL;
    if (presentModeCount != 0) {
        presentModes = (VkPresentModeKHR*)RL_MALLOC(presentModeCount * sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModeCount, presentModes);
    } else {
        TRACELOG(LOG_FATAL, "RLVK: No present modes found for swapchain creation.");
        RL_FREE(formats);
        vkDestroyDevice(vkDevice, NULL); vkDevice = VK_NULL_HANDLE;
        return;
    }

    // Choose swap surface format
    VkSurfaceFormatKHR surfaceFormat = formats[0]; // Default to first available
    for (uint32_t i = 0; i < formatCount; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = formats[i];
            break;
        }
    }
    vkSwapchainImageFormat = surfaceFormat.format;
    TRACELOG(LOG_INFO, "RLVK: Chosen swapchain format: %d, color space: %d", surfaceFormat.format, surfaceFormat.colorSpace);

    // Choose swap present mode
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // Guaranteed to be available
    for (uint32_t i = 0; i < presentModeCount; i++) {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = presentModes[i];
            break;
        }
    }
    TRACELOG(LOG_INFO, "RLVK: Chosen present mode: %d", presentMode);

    RL_FREE(formats);
    RL_FREE(presentModes);

    // Choose swap extent
    if (capabilities.currentExtent.width != UINT32_MAX) {
        vkSwapchainExtent = capabilities.currentExtent;
    } else {
        vkSwapchainExtent.width = (uint32_t)screenWidth;
        vkSwapchainExtent.height = (uint32_t)screenHeight;
        vkSwapchainExtent.width = MAX(capabilities.minImageExtent.width, MIN(capabilities.maxImageExtent.width, vkSwapchainExtent.width));
        vkSwapchainExtent.height = MAX(capabilities.minImageExtent.height, MIN(capabilities.maxImageExtent.height, vkSwapchainExtent.height));
    }
    TRACELOG(LOG_INFO, "RLVK: Swapchain extent: %u x %u", vkSwapchainExtent.width, vkSwapchainExtent.height);

    vkSwapchainImageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && vkSwapchainImageCount > capabilities.maxImageCount) {
        vkSwapchainImageCount = capabilities.maxImageCount;
    }
    TRACELOG(LOG_INFO, "RLVK: Swapchain image count: %u", vkSwapchainImageCount);

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {0};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = vkSurface;
    swapchainCreateInfo.minImageCount = vkSwapchainImageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = vkSwapchainExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // For rendering directly to swapchain images

    uint32_t qFamilyIndices[] = {queueFamilyIndices.graphicsFamily, queueFamilyIndices.presentFamily};
    if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = qFamilyIndices;
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0; // Optional
        swapchainCreateInfo.pQueueFamilyIndices = NULL; // Optional
    }

    swapchainCreateInfo.preTransform = capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // No alpha blending with window system
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE; // Allow clipping if other windows obscure parts of the surface
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE; // For resizing, not used now

    result = vkCreateSwapchainKHR(vkDevice, &swapchainCreateInfo, NULL, &vkSwapchain);
    if (result != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create swapchain (Error: %i)", result);
        vkDestroyDevice(vkDevice, NULL); vkDevice = VK_NULL_HANDLE;
        return;
    }
    TRACELOG(LOG_INFO, "RLVK: Swapchain created successfully.");

    // Get swapchain images
    // vkSwapchainImageCount was requested, now query actual count (can be higher)
    vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &vkSwapchainImageCount, NULL);
    vkSwapchainImages = (VkImage*)RL_MALLOC(vkSwapchainImageCount * sizeof(VkImage));
    if (!vkSwapchainImages) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to allocate memory for swapchain images.");
        vkDestroySwapchainKHR(vkDevice, vkSwapchain, NULL); vkSwapchain = VK_NULL_HANDLE;
        vkDestroyDevice(vkDevice, NULL); vkDevice = VK_NULL_HANDLE;
        return;
    }
    vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &vkSwapchainImageCount, vkSwapchainImages);
    TRACELOG(LOG_INFO, "RLVK: Retrieved %u swapchain images.", vkSwapchainImageCount);

    // --- Image View Creation (Step 5) ---
    vkSwapchainImageViews = (VkImageView*)RL_MALLOC(vkSwapchainImageCount * sizeof(VkImageView));
    if (!vkSwapchainImageViews) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to allocate memory for swapchain image views.");
        // Perform necessary cleanup from previous steps
        RL_FREE(vkSwapchainImages); vkSwapchainImages = NULL;
        vkDestroySwapchainKHR(vkDevice, vkSwapchain, NULL); vkSwapchain = VK_NULL_HANDLE;
        vkDestroyDevice(vkDevice, NULL); vkDevice = VK_NULL_HANDLE;
        return;
    }

    for (uint32_t i = 0; i < vkSwapchainImageCount; i++) {
        VkImageViewCreateInfo viewInfo = {0};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = vkSwapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = vkSwapchainImageFormat;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView(vkDevice, &viewInfo, NULL, &vkSwapchainImageViews[i]);
        if (result != VK_SUCCESS) {
            TRACELOG(LOG_FATAL, "RLVK: Failed to create image view %u (Error: %i)", i, result);
            // Perform cleanup
            for (uint32_t j = 0; j < i; j++) vkDestroyImageView(vkDevice, vkSwapchainImageViews[j], NULL);
            RL_FREE(vkSwapchainImageViews); vkSwapchainImageViews = NULL;
            RL_FREE(vkSwapchainImages); vkSwapchainImages = NULL;
            vkDestroySwapchainKHR(vkDevice, vkSwapchain, NULL); vkSwapchain = VK_NULL_HANDLE;
            vkDestroyDevice(vkDevice, NULL); vkDevice = VK_NULL_HANDLE;
            return;
        }
    }
    TRACELOG(LOG_INFO, "RLVK: Swapchain image views created successfully.");

    // --- Render Pass Creation (Step 6) ---
    // Find supported depth format (helper function recommended)
    // VkFormat findSupportedFormat(const VkFormat* candidates, uint32_t candidateCount, VkImageTiling tiling, VkFormatFeatureFlags features);
    // For now, assume VK_FORMAT_D32_SFLOAT is supported. A real implementation needs findSupportedFormat.
    vkDepthFormat = VK_FORMAT_D32_SFLOAT; // Placeholder - must be queried

    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = vkSwapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment = {0};
    depthAttachment.format = vkDepthFormat; // Must be a supported depth format
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef = {0};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {0};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = sizeof(attachments) / sizeof(VkAttachmentDescription);
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    result = vkCreateRenderPass(vkDevice, &renderPassInfo, NULL, &vkRenderPass);
    if (result != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create render pass (Error: %i)", result);
        // Perform cleanup... (image views, swapchain, device etc.)
        return; // Simplified cleanup for now
    }
    TRACELOG(LOG_INFO, "RLVK: Render pass created successfully.");

    // --- Depth Resources Creation (Step 7) ---
    VkImageCreateInfo imageInfo = {0};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = vkSwapchainExtent.width;
    imageInfo.extent.height = vkSwapchainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = vkDepthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // For best performance
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    result = vkCreateImage(vkDevice, &imageInfo, NULL, &vkDepthImage);
    if (result != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create depth image (Error: %i)", result);
        // Cleanup...
        return;
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vkDevice, vkDepthImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    // allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // Helper needed
    // For now, assume memoryTypeIndex 0 is valid (highly unlikely in real scenario without querying)
    // This needs a proper findMemoryType implementation.
    uint32_t memoryTypeIndex = 0; // Placeholder - THIS IS A BUG without proper findMemoryType
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memProperties);
    bool memoryTypeFound = false;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
            memoryTypeIndex = i;
            memoryTypeFound = true;
            break;
        }
    }
    if (!memoryTypeFound) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to find suitable memory type for depth image!");
        // Cleanup...
        return;
    }
    allocInfo.memoryTypeIndex = memoryTypeIndex;


    result = vkAllocateMemory(vkDevice, &allocInfo, NULL, &vkDepthImageMemory);
    if (result != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to allocate depth image memory (Error: %i)", result);
        // Cleanup...
        return;
    }
    vkBindImageMemory(vkDevice, vkDepthImage, vkDepthImageMemory, 0);

    VkImageViewCreateInfo depthViewInfo = {0};
    depthViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depthViewInfo.image = vkDepthImage;
    depthViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthViewInfo.format = vkDepthFormat;
    depthViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthViewInfo.subresourceRange.baseMipLevel = 0;
    depthViewInfo.subresourceRange.levelCount = 1;
    depthViewInfo.subresourceRange.baseArrayLayer = 0;
    depthViewInfo.subresourceRange.layerCount = 1;

    result = vkCreateImageView(vkDevice, &depthViewInfo, NULL, &vkDepthImageView);
    if (result != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create depth image view (Error: %i)", result);
        // Cleanup...
        return;
    }
    TRACELOG(LOG_INFO, "RLVK: Depth resources created successfully.");

    // --- Framebuffer Creation (Step 8) ---
    vkFramebuffers = (VkFramebuffer*)RL_MALLOC(vkSwapchainImageCount * sizeof(VkFramebuffer));
    if (!vkFramebuffers) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to allocate memory for framebuffers.");
        // Perform cleanup... (this is getting repetitive, a helper might be good)
        return;
    }
    for (uint32_t i = 0; i < vkSwapchainImageCount; i++) {
        VkImageView attachments[] = {
            vkSwapchainImageViews[i],
            vkDepthImageView
        };

        VkFramebufferCreateInfo framebufferInfo = {0};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vkRenderPass;
        framebufferInfo.attachmentCount = sizeof(attachments) / sizeof(VkImageView);
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vkSwapchainExtent.width;
        framebufferInfo.height = vkSwapchainExtent.height;
        framebufferInfo.layers = 1;

        result = vkCreateFramebuffer(vkDevice, &framebufferInfo, NULL, &vkFramebuffers[i]);
        if (result != VK_SUCCESS) {
            TRACELOG(LOG_FATAL, "RLVK: Failed to create framebuffer %u (Error: %i)", i, result);
            // Perform cleanup...
            return;
        }
    }
    TRACELOG(LOG_INFO, "RLVK: Framebuffers created successfully.");

    // --- Command Pool and Command Buffers (Step 9) ---
    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    result = vkCreateCommandPool(vkDevice, &poolInfo, NULL, &vkCommandPool);
    if (result != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create command pool (Error: %i)", result);
        // Perform cleanup...
        return;
    }
    TRACELOG(LOG_INFO, "RLVK: Command pool created successfully.");

    vkCommandBuffers = (VkCommandBuffer*)RL_MALLOC(vkSwapchainImageCount * sizeof(VkCommandBuffer));
    if(!vkCommandBuffers) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to allocate memory for command buffers.");
        // Perform cleanup...
        return;
    }

    VkCommandBufferAllocateInfo cmdAllocInfo = {0};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.commandPool = vkCommandPool;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandBufferCount = vkSwapchainImageCount; // Allocate all at once

    result = vkAllocateCommandBuffers(vkDevice, &cmdAllocInfo, vkCommandBuffers);
    if (result != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to allocate command buffers (Error: %i)", result);
        // Perform cleanup...
        return;
    }
    TRACELOG(LOG_INFO, "RLVK: Command buffers allocated successfully.");

    // --- Synchronization Primitives (Step 10) ---
    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Create fences in signaled state

    vkInFlightFences = (VkFence*)RL_MALLOC(vkSwapchainImageCount * sizeof(VkFence));
    if (!vkInFlightFences) {
         TRACELOG(LOG_FATAL, "RLVK: Failed to allocate memory for in-flight fences.");
        // Perform cleanup...
        return;
    }

    if (vkCreateSemaphore(vkDevice, &semaphoreInfo, NULL, &vkImageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(vkDevice, &semaphoreInfo, NULL, &vkRenderFinishedSemaphore) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create semaphores.");
        // Perform cleanup...
        if (vkImageAvailableSemaphore != VK_NULL_HANDLE) vkDestroySemaphore(vkDevice, vkImageAvailableSemaphore, NULL);
        if (vkRenderFinishedSemaphore != VK_NULL_HANDLE) vkDestroySemaphore(vkDevice, vkRenderFinishedSemaphore, NULL);
        RL_FREE(vkInFlightFences); vkInFlightFences = NULL;
        return;
    }

    for (uint32_t i = 0; i < vkSwapchainImageCount; i++) {
        if (vkCreateFence(vkDevice, &fenceInfo, NULL, &vkInFlightFences[i]) != VK_SUCCESS) {
            TRACELOG(LOG_FATAL, "RLVK: Failed to create fence %u.", i);
            // Perform cleanup for already created fences and semaphores
            for(uint32_t j=0; j < i; ++j) vkDestroyFence(vkDevice, vkInFlightFences[j], NULL);
            RL_FREE(vkInFlightFences); vkInFlightFences = NULL;
            vkDestroySemaphore(vkDevice, vkImageAvailableSemaphore, NULL);
            vkDestroySemaphore(vkDevice, vkRenderFinishedSemaphore, NULL);
            return;
        }
    }
    TRACELOG(LOG_INFO, "RLVK: Synchronization primitives created successfully.");

    rlvkInitializeVertexBuffer(); // Initialize CPU vertex buffer

    // Initialize GPU vertex buffers
    rlvkGPUVertexBufferSize = RLVK_DEFAULT_CPU_VERTEX_BUFFER_CAPACITY * sizeof(rlvkVertex);
    rlvkGPUVertexBuffers = (rlvkBuffer*)RL_MALLOC(vkSwapchainImageCount * sizeof(rlvkBuffer));
    if (!rlvkGPUVertexBuffers) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to allocate memory for GPU vertex buffers array.");
        // Perform necessary cleanup...
        rlvkReady = false;
        return;
    }

    for (uint32_t i = 0; i < vkSwapchainImageCount; i++) {
        if (!rlvkCreateBuffer(rlvkGPUVertexBufferSize,
                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                              &rlvkGPUVertexBuffers[i].buffer,
                              &rlvkGPUVertexBuffers[i].memory)) {
            TRACELOG(LOG_FATAL, "RLVK: Failed to create GPU vertex buffer for frame %u.", i);
            // Cleanup already created buffers
            for (uint32_t j = 0; j < i; j++) {
                vkDestroyBuffer(vkDevice, rlvkGPUVertexBuffers[j].buffer, NULL);
                vkFreeMemory(vkDevice, rlvkGPUVertexBuffers[j].memory, NULL);
            }
            RL_FREE(rlvkGPUVertexBuffers);
            rlvkGPUVertexBuffers = NULL;
            // Perform other necessary cleanup...
            rlvkReady = false;
            return;
        }
        TRACELOG(LOG_INFO, "RLVK: GPU vertex buffer %u created (Size: %lu bytes).", i, rlvkGPUVertexBufferSize);
    }

    // Create Shader Modules
    // IMPORTANT: Using placeholder SPIR-V data. Replace with actual compiled bytecode.
    vkVertShaderModule = rlvkCreateShaderModule(shapes_vert_spv_placeholder, sizeof(shapes_vert_spv_placeholder));
    vkFragShaderModule = rlvkCreateShaderModule(shapes_frag_spv_placeholder, sizeof(shapes_frag_spv_placeholder));

    if (vkVertShaderModule == VK_NULL_HANDLE || vkFragShaderModule == VK_NULL_HANDLE) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create shader modules.");
        // Perform necessary cleanup of already created resources...
        rlvkReady = false;
        return;
    }

    // Create Descriptor Set Layout (for texture sampler)
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {0};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = NULL;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {0};
    descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutInfo.bindingCount = 1;
    descriptorSetLayoutInfo.pBindings = &samplerLayoutBinding;

    if (vkCreateDescriptorSetLayout(vkDevice, &descriptorSetLayoutInfo, NULL, &vkDescriptorSetLayout) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create descriptor set layout!");
        // Perform necessary cleanup...
        rlvkReady = false;
        return;
    }

    // Create Pipeline Layout
    VkPushConstantRange pushConstantRange = {0};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(float[16]); // sizeof(Matrix)

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &vkDescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, NULL, &vkPipelineLayout) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create pipeline layout!");
        // Perform necessary cleanup...
        rlvkReady = false;
        return;
    }

    // Create Graphics Pipeline
    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vkVertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = vkFragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkVertexInputBindingDescription bindingDescription = {0};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(rlvkVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescriptions[3];
    // Position
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(rlvkVertex, position);
    // TexCoord
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(rlvkVertex, texcoord);
    // Color
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM; // Matches shader expectation (normalized ubyte)
    attributeDescriptions[2].offset = offsetof(rlvkVertex, color);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = sizeof(attributeDescriptions) / sizeof(VkVertexInputAttributeDescription);
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Default, can be dynamic later
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState = {0};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE; // VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // VK_FRONT_FACE_CLOCKWISE if flipping Y
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {0};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {0};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState = {0};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState);
    dynamicState.pDynamicStates = dynamicStates;

    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = vkPipelineLayout;
    pipelineInfo.renderPass = vkRenderPass;
    pipelineInfo.subpass = 0;

    if (vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &vkGraphicsPipeline) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create graphics pipeline!");
        // Perform necessary cleanup...
        rlvkReady = false;
        return;
    }

    // Create Default Texture, Sampler, and Descriptor Set
    // 1. Create Sampler
    VkSamplerCreateInfo samplerInfo = {0};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    if (vkCreateSampler(vkDevice, &samplerInfo, NULL, &vkDefaultTextureSampler) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create default texture sampler!");
        // Perform necessary cleanup...
        rlvkReady = false;
        return;
    }

    // 2. Create Image and Staging Buffer
    unsigned char pixels[4] = {255, 255, 255, 255};
    VkDeviceSize imageSize = sizeof(pixels);
    rlvkBuffer stagingBuffer;

    if (!rlvkCreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          &stagingBuffer.buffer, &stagingBuffer.memory)) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create staging buffer for default texture!");
        // Perform necessary cleanup...
        rlvkReady = false;
        return;
    }

    void* data;
    vkMapMemory(vkDevice, stagingBuffer.memory, 0, imageSize, 0, &data);
    memcpy(data, pixels, (size_t)imageSize);
    vkUnmapMemory(vkDevice, stagingBuffer.memory);

    VkImageCreateInfo imageCreateInfo = {0};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = 1;
    imageCreateInfo.extent.height = 1;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // Or _SRGB if color space transformations are handled
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(vkDevice, &imageCreateInfo, NULL, &vkDefaultTextureImage) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create default texture image!");
        // Perform necessary cleanup...
        rlvkReady = false;
        return;
    }

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(vkDevice, vkDefaultTextureImage, &memReqs);
    VkMemoryAllocateInfo allocInfoImage = {0};
    allocInfoImage.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfoImage.allocationSize = memReqs.size;
    allocInfoImage.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (vkAllocateMemory(vkDevice, &allocInfoImage, NULL, &vkDefaultTextureImageMemory) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to allocate default texture image memory!");
        // Perform necessary cleanup...
        rlvkReady = false;
        return;
    }
    vkBindImageMemory(vkDevice, vkDefaultTextureImage, vkDefaultTextureImageMemory, 0);

    // 3. Transition Image Layout and Copy
    // Define a lambda or local function to record commands for this specific operation
    void record_default_texture_init_cmds_with_copy(VkCommandBuffer cmdBuf) {
        // Transition UNDEFINED -> TRANSFER_DST
        VkImageMemoryBarrier barrier = {0};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = vkDefaultTextureImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

        // Copy Buffer to Image
        VkBufferImageCopy region = {0};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = (VkOffset3D){0, 0, 0};
        region.imageExtent = (VkExtent3D){1, 1, 1};
        vkCmdCopyBufferToImage(cmdBuf, stagingBuffer.buffer, vkDefaultTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        // Transition TRANSFER_DST -> SHADER_READ_ONLY
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
    }
    rlvkRecordAndSubmitCommandBuffer(record_default_texture_init_cmds_with_copy, "default texture initialization");

    // Cleanup staging buffer
    vkDestroyBuffer(vkDevice, stagingBuffer.buffer, NULL);
    vkFreeMemory(vkDevice, stagingBuffer.memory, NULL);

    // 4. Create Image View
    VkImageViewCreateInfo viewInfo = {0};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = vkDefaultTextureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // Must match image format
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(vkDevice, &viewInfo, NULL, &vkDefaultTextureImageView) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create default texture image view!");
        // Perform necessary cleanup...
        rlvkReady = false;
        return;
    }

    // Cleanup staging buffer
    vkDestroyBuffer(vkDevice, stagingBuffer.buffer, NULL);
    vkFreeMemory(vkDevice, stagingBuffer.memory, NULL);


    // 5. Create Descriptor Pool
    VkDescriptorPoolSize poolSize = {0};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1; // Enough for the default texture
    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1; // Enough for the default texture
    if (vkCreateDescriptorPool(vkDevice, &poolInfo, NULL, &vkDescriptorPool) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to create descriptor pool!");
        // Perform necessary cleanup...
        rlvkReady = false;
        return;
    }

    // 6. Allocate and Update Default Descriptor Set
    VkDescriptorSetAllocateInfo descAllocInfo = {0};
    descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descAllocInfo.descriptorPool = vkDescriptorPool;
    descAllocInfo.descriptorSetCount = 1;
    descAllocInfo.pSetLayouts = &vkDescriptorSetLayout; // From pipeline creation
    if (vkAllocateDescriptorSets(vkDevice, &descAllocInfo, &vkDefaultDescriptorSet) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to allocate default descriptor set!");
        // Perform necessary cleanup...
        rlvkReady = false;
        return;
    }

    VkDescriptorImageInfo imageBindingInfo = {0};
    imageBindingInfo.sampler = vkDefaultTextureSampler;
    imageBindingInfo.imageView = vkDefaultTextureImageView;
    imageBindingInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // Ensure this is the layout after transition

    VkWriteDescriptorSet descriptorWrite = {0};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = vkDefaultDescriptorSet;
    descriptorWrite.dstBinding = 0; // Matches shader binding
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageBindingInfo;
    vkUpdateDescriptorSets(vkDevice, 1, &descriptorWrite, 0, NULL);


    rlvkReady = true; // All initialization steps completed successfully
    TRACELOG(LOG_INFO, "RLVK: Vulkan backend initialized successfully.");
}

// Helper function to find a suitable memory type for buffer allocation
static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    TRACELOG(LOG_FATAL, "RLVK: Failed to find suitable memory type!");
    // This is a critical error, should probably throw or handle more gracefully
    return UINT32_MAX; // Should not happen if logic is correct and device is suitable
}


// Helper function to create a Vulkan buffer and allocate its memory
static bool rlvkCreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(vkDevice, &bufferInfo, NULL, buffer) != VK_SUCCESS) {
        TRACELOG(LOG_ERROR, "RLVK: Failed to create buffer.");
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(vkDevice, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (allocInfo.memoryTypeIndex == UINT32_MAX) { // Check if findMemoryType failed
        TRACELOG(LOG_ERROR, "RLVK: findMemoryType failed, cannot allocate buffer memory.");
        vkDestroyBuffer(vkDevice, *buffer, NULL); // Clean up the created buffer handle
        *buffer = VK_NULL_HANDLE;
        return false;
    }

    if (vkAllocateMemory(vkDevice, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
        TRACELOG(LOG_ERROR, "RLVK: Failed to allocate buffer memory.");
        vkDestroyBuffer(vkDevice, *buffer, NULL); // Clean up the created buffer handle
        *buffer = VK_NULL_HANDLE;
        return false;
    }

    if (vkBindBufferMemory(vkDevice, *buffer, *bufferMemory, 0) != VK_SUCCESS) {
        TRACELOG(LOG_ERROR, "RLVK: Failed to bind buffer memory.");
        vkFreeMemory(vkDevice, *bufferMemory, NULL); // Clean up allocated memory
        vkDestroyBuffer(vkDevice, *buffer, NULL);    // Clean up the created buffer handle
        *buffer = VK_NULL_HANDLE;
        *bufferMemory = VK_NULL_HANDLE;
        return false;
    }
    return true;
}


void rlvkClose(void) {
    TRACELOG(LOG_INFO, "RLVK: Closing Vulkan backend.");

    rlvkDestroyVertexBuffer(); // Destroy CPU vertex buffer

    if (vkDevice != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(vkDevice); // Ensure device is idle before destroying resources
    }

    // Destroy GPU vertex buffers
    if (rlvkGPUVertexBuffers != NULL) {
        for (uint32_t i = 0; i < vkSwapchainImageCount; i++) {
            if (rlvkGPUVertexBuffers[i].buffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(vkDevice, rlvkGPUVertexBuffers[i].buffer, NULL);
            }
            if (rlvkGPUVertexBuffers[i].memory != VK_NULL_HANDLE) {
                vkFreeMemory(vkDevice, rlvkGPUVertexBuffers[i].memory, NULL);
            }
        }
        RL_FREE(rlvkGPUVertexBuffers);
        rlvkGPUVertexBuffers = NULL;
        TRACELOG(LOG_DEBUG, "RLVK: GPU vertex buffers destroyed.");
    }

    if (vkGraphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(vkDevice, vkGraphicsPipeline, NULL);
        vkGraphicsPipeline = VK_NULL_HANDLE;
    }
    if (vkPipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
        vkPipelineLayout = VK_NULL_HANDLE;
    }
    if (vkDescriptorSetLayout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, NULL);
        vkDescriptorSetLayout = VK_NULL_HANDLE;
    }
    if (vkFragShaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(vkDevice, vkFragShaderModule, NULL);
        vkFragShaderModule = VK_NULL_HANDLE;
    }
    if (vkVertShaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(vkDevice, vkVertShaderModule, NULL);
        vkVertShaderModule = VK_NULL_HANDLE;
    }

    // Cleanup default texture resources
    if (vkDefaultTextureSampler != VK_NULL_HANDLE) {
        vkDestroySampler(vkDevice, vkDefaultTextureSampler, NULL);
        vkDefaultTextureSampler = VK_NULL_HANDLE;
    }
    if (vkDefaultTextureImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(vkDevice, vkDefaultTextureImageView, NULL);
        vkDefaultTextureImageView = VK_NULL_HANDLE;
    }
    if (vkDefaultTextureImage != VK_NULL_HANDLE) {
        vkDestroyImage(vkDevice, vkDefaultTextureImage, NULL);
        vkDefaultTextureImage = VK_NULL_HANDLE;
    }
    if (vkDefaultTextureImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(vkDevice, vkDefaultTextureImageMemory, NULL);
        vkDefaultTextureImageMemory = VK_NULL_HANDLE;
    }
    if (vkDescriptorPool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);
        vkDescriptorPool = VK_NULL_HANDLE;
    }
    // vkDefaultDescriptorSet is freed when the pool is destroyed

    if (vkImageAvailableSemaphore != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(vkDevice); // Ensure device is idle before destroying resources
    }

    if (vkImageAvailableSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(vkDevice, vkImageAvailableSemaphore, NULL);
        vkImageAvailableSemaphore = VK_NULL_HANDLE;
    }
    if (vkRenderFinishedSemaphore != VK_NULL_HANDLE) {
        vkDestroySemaphore(vkDevice, vkRenderFinishedSemaphore, NULL);
        vkRenderFinishedSemaphore = VK_NULL_HANDLE;
    }
    if (vkInFlightFences != NULL) {
        // Use MAX_FRAMES_IN_FLIGHT or actual count used for fences if different from vkSwapchainImageCount
        uint32_t fenceCount = vkSwapchainImageCount; // Assuming fences per swapchain image for now
        for (uint32_t i = 0; i < fenceCount; i++) {
            if (vkInFlightFences[i] != VK_NULL_HANDLE) {
                 vkDestroyFence(vkDevice, vkInFlightFences[i], NULL);
            }
        }
        RL_FREE(vkInFlightFences);
        vkInFlightFences = NULL;
        TRACELOG(LOG_DEBUG, "RLVK: Fences destroyed.");
    }

    if (vkCommandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
        vkCommandPool = VK_NULL_HANDLE;
        TRACELOG(LOG_DEBUG, "RLVK: Command pool destroyed.");
    }
     if (vkCommandBuffers != NULL) { // Command buffers are freed with the pool
        RL_FREE(vkCommandBuffers);
        vkCommandBuffers = NULL;
    }

    if (vkFramebuffers != NULL) {
        for (uint32_t i = 0; i < vkSwapchainImageCount; i++) {
            if (vkFramebuffers[i] != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(vkDevice, vkFramebuffers[i], NULL);
            }
        }
        RL_FREE(vkFramebuffers);
        vkFramebuffers = NULL;
        TRACELOG(LOG_DEBUG, "RLVK: Framebuffers destroyed.");
    }

    if (vkDepthImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(vkDevice, vkDepthImageView, NULL);
        vkDepthImageView = VK_NULL_HANDLE;
    }
    if (vkDepthImage != VK_NULL_HANDLE) {
        vkDestroyImage(vkDevice, vkDepthImage, NULL);
        vkDepthImage = VK_NULL_HANDLE;
    }
    if (vkDepthImageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(vkDevice, vkDepthImageMemory, NULL);
        vkDepthImageMemory = VK_NULL_HANDLE;
    }
    TRACELOG(LOG_DEBUG, "RLVK: Depth resources destroyed.");

    if (vkRenderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;
        TRACELOG(LOG_DEBUG, "RLVK: Render pass destroyed.");
    }

    if (vkSwapchainImageViews != NULL) {
        for (uint32_t i = 0; i < vkSwapchainImageCount; i++) {
            if (vkSwapchainImageViews[i] != VK_NULL_HANDLE) {
                vkDestroyImageView(vkDevice, vkSwapchainImageViews[i], NULL);
            }
        }
        RL_FREE(vkSwapchainImageViews);
        vkSwapchainImageViews = NULL;
        TRACELOG(LOG_DEBUG, "RLVK: Swapchain image views destroyed.");
    }

    if (vkSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(vkDevice, vkSwapchain, NULL);
        vkSwapchain = VK_NULL_HANDLE;
        TRACELOG(LOG_DEBUG, "RLVK: Swapchain destroyed.");
    }
    if (vkSwapchainImages != NULL) {
        RL_FREE(vkSwapchainImages);
        vkSwapchainImages = NULL;
    }

    vkGraphicsQueue = VK_NULL_HANDLE;
    vkPresentQueue = VK_NULL_HANDLE;
    if (vkDevice != VK_NULL_HANDLE) {
        vkDestroyDevice(vkDevice, NULL);
        TRACELOG(LOG_DEBUG,"RLVK: Logical device destroyed.");
    }
    vkDevice = VK_NULL_HANDLE;
    vkPhysicalDevice = VK_NULL_HANDLE;

    vkSurface = VK_NULL_HANDLE;
    vkInstance = VK_NULL_HANDLE;

    memset(&queueFamilyIndices, 0, sizeof(QueueFamilyIndices));
    screenWidth = 0;
    screenHeight = 0;
    currentFrame = 0; // Reset frame counter

    rlvkReady = false;
    TRACELOG(LOG_INFO, "RLVK: Vulkan backend resources reset.");
}

bool rlvkIsReady(void) {
    return rlvkReady;
}

void rlvkInitializeVertexBuffer(void) {
    rlvkCPUVertexBuffer = (rlvkVertex *)RL_MALLOC(RLVK_DEFAULT_CPU_VERTEX_BUFFER_CAPACITY * sizeof(rlvkVertex));
    if (rlvkCPUVertexBuffer == NULL) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to allocate CPU vertex buffer.");
        // This is a critical error, a real application might try to recover or exit.
        rlvkCPUVertexBufferCapacity = 0;
        rlvkCPUVertexCount = 0;
        return;
    }
    rlvkCPUVertexBufferCapacity = RLVK_DEFAULT_CPU_VERTEX_BUFFER_CAPACITY;
    rlvkCPUVertexCount = 0;
    TRACELOG(LOG_INFO, "RLVK: CPU vertex buffer initialized (Capacity: %u vertices)", rlvkCPUVertexBufferCapacity);
}

void rlvkResizeVertexBuffer(void) {
    if (rlvkCPUVertexBuffer == NULL) { // Should not happen if rlvkInitializeVertexBuffer was called
        TRACELOG(LOG_WARNING, "RLVK: Attempted to resize a NULL CPU vertex buffer. Initializing instead.");
        rlvkInitializeVertexBuffer();
        return;
    }

    uint32_t newCapacity = rlvkCPUVertexBufferCapacity * 2;
    // Consider adding a maximum capacity check if necessary
    // if (newCapacity > SOME_ABSOLUTE_MAX_CAPACITY) newCapacity = SOME_ABSOLUTE_MAX_CAPACITY;
    // if (newCapacity <= rlvkCPUVertexBufferCapacity) { // Overflow or already at max
    //     TRACELOG(LOG_ERROR, "RLVK: Failed to resize CPU vertex buffer (max capacity reached or overflow).");
    //     return;
    // }


    rlvkVertex *newBuffer = (rlvkVertex *)RL_REALLOC(rlvkCPUVertexBuffer, newCapacity * sizeof(rlvkVertex));
    if (newBuffer == NULL) {
        TRACELOG(LOG_ERROR, "RLVK: Failed to reallocate CPU vertex buffer (New Capacity: %u). Current data preserved.", newCapacity);
        // Keep using the old buffer if reallocation fails.
        return;
    }
    rlvkCPUVertexBuffer = newBuffer;
    rlvkCPUVertexBufferCapacity = newCapacity;
    TRACELOG(LOG_INFO, "RLVK: CPU vertex buffer resized (New Capacity: %u vertices)", rlvkCPUVertexBufferCapacity);
}

void rlvkResetVertexBuffer(void) {
    rlvkCPUVertexCount = 0;
    // TRACELOG(LOG_DEBUG, "RLVK: CPU vertex buffer reset (count cleared)."); // Can be noisy
}

void rlvkDestroyVertexBuffer(void) {
    if (rlvkCPUVertexBuffer != NULL) {
        RL_FREE(rlvkCPUVertexBuffer);
        rlvkCPUVertexBuffer = NULL;
    }
    rlvkCPUVertexBufferCapacity = 0;
    rlvkCPUVertexCount = 0;
    TRACELOG(LOG_INFO, "RLVK: CPU vertex buffer destroyed.");
}


void rlvkSetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    currentColorR = r;
    currentColorG = g;
    currentColorB = b;
    currentColorA = a;
}

void rlvkSetTexCoord(float x, float y) {
    currentTexcoordX = x;
    currentTexcoordY = y;
}

void rlvkAddVertex(float x, float y, float z) {
    if (rlvkCPUVertexBuffer == NULL) {
        TRACELOG(LOG_WARNING, "RLVK: Attempted to add vertex to NULL buffer. Initializing buffer.");
        rlvkInitializeVertexBuffer();
        if (rlvkCPUVertexBuffer == NULL) return; // Initialization failed
    }

    if (rlvkCPUVertexCount >= rlvkCPUVertexBufferCapacity) {
        TRACELOG(LOG_DEBUG, "RLVK: CPU vertex buffer full (Count: %u, Capacity: %u). Resizing.", rlvkCPUVertexCount, rlvkCPUVertexBufferCapacity);
        rlvkResizeVertexBuffer();
        if (rlvkCPUVertexCount >= rlvkCPUVertexBufferCapacity) { // Check if resize failed or was insufficient
            TRACELOG(LOG_ERROR, "RLVK: Failed to add vertex, buffer resize unsuccessful or insufficient.");
            return;
        }
    }

    rlvkVertex *vertex = &rlvkCPUVertexBuffer[rlvkCPUVertexCount];

    vertex->position[0] = x;
    vertex->position[1] = y;
    vertex->position[2] = z;

    vertex->texcoord[0] = currentTexcoordX;
    vertex->texcoord[1] = currentTexcoordY;

    vertex->color[0] = currentColorR;
    vertex->color[1] = currentColorG;
    vertex->color[2] = currentColorB;
    vertex->color[3] = currentColorA;

    rlvkCPUVertexCount++;
}


void rlvkBeginDrawing(void) {
    if (!rlvkReady) return;

    rlvkResetVertexBuffer(); // Reset vertex count for the new frame/batch

    // Wait for the fence of the current frame to ensure the command buffer is free to be reused
    // MAX_FRAMES_IN_FLIGHT should be used here instead of vkSwapchainImageCount if they can differ.
    // For this implementation, we assume they are the same (fences per swapchain image).
    VkResult fenceResult = vkWaitForFences(vkDevice, 1, &vkInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    if (fenceResult != VK_SUCCESS) {
        TRACELOG(LOG_ERROR, "RLVK: Failed to wait for fence (Error: %i)", fenceResult);
        // Handle error, possibly by trying to recreate resources or exiting
        return;
    }

    // Acquire next image from swapchain
    VkResult acquireResult = vkAcquireNextImageKHR(vkDevice, vkSwapchain, UINT64_MAX, vkImageAvailableSemaphore, VK_NULL_HANDLE, &acquiredImageIndex);

    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        TRACELOG(LOG_WARNING, "RLVK: Swapchain out of date during vkAcquireNextImageKHR. TODO: Recreate swapchain.");
        // rlRecreateSwapchain(); // Placeholder for swapchain recreation logic
        return; // Skip rendering this frame
    } else if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to acquire swapchain image (Error: %i)", acquireResult);
        return; // Skip rendering this frame
    }

    // Only reset the fence if we are sure we will submit work with it.
    // This happens after successfully acquiring an image.
    vkResetFences(vkDevice, 1, &vkInFlightFences[currentFrame]);


    // Begin Command Buffer
    VkCommandBuffer currentCmdBuffer = vkCommandBuffers[acquiredImageIndex]; // Use command buffer corresponding to acquired image
    // Or use vkCommandBuffers[currentFrame] if MAX_FRAMES_IN_FLIGHT is less than swapchain image count.
    // For now, assume one command buffer per swapchain image.

    // vkResetCommandBuffer(currentCmdBuffer, 0); // Not needed if VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT is set on pool

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(currentCmdBuffer, &beginInfo) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to begin command buffer.");
        return;
    }

    // Begin Render Pass
    VkClearValue clearValues[2];
    clearValues[0].color = currentClearColor; // Use the color set by rlvkClearBackground
    clearValues[1].depthStencil = defaultDepthStencilClear;

    VkRenderPassBeginInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkRenderPass;
    renderPassInfo.framebuffer = vkFramebuffers[acquiredImageIndex];
    renderPassInfo.renderArea.offset = (VkOffset2D){0, 0};
    renderPassInfo.renderArea.extent = vkSwapchainExtent;
    renderPassInfo.clearValueCount = sizeof(clearValues) / sizeof(VkClearValue);
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(currentCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Set Dynamic Viewport and Scissor
    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f; // Or (float)vkSwapchainExtent.height and negative height if flipping
    viewport.width = (float)vkSwapchainExtent.width;
    viewport.height = (float)vkSwapchainExtent.height; // Or -(float)vkSwapchainExtent.height if flipping
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(currentCmdBuffer, 0, 1, &viewport);

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = vkSwapchainExtent;
    vkCmdSetScissor(currentCmdBuffer, 0, 1, &scissor);

    // Bind Graphics Pipeline
    if (vkGraphicsPipeline != VK_NULL_HANDLE) {
        vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkGraphicsPipeline);
    } else {
        TRACELOG(LOG_WARNING, "RLVK: Graphics pipeline not available for binding in rlvkBeginDrawing.");
    }

    // Bind the vertex buffer for the current frame
    // NOTE: This assumes one vertex buffer per swapchain image, indexed by acquiredImageIndex.
    // If MAX_FRAMES_IN_FLIGHT is different, currentFrame might be more appropriate.
    if (rlvkGPUVertexBuffers != NULL && rlvkGPUVertexBuffers[acquiredImageIndex].buffer != VK_NULL_HANDLE) {
        VkBuffer currentVkBuffer = rlvkGPUVertexBuffers[acquiredImageIndex].buffer;
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(currentCmdBuffer, 0, 1, &currentVkBuffer, offsets);
    } else {
        TRACELOG(LOG_WARNING, "RLVK: GPU vertex buffer not available for binding in rlvkBeginDrawing.");
    }

    // Bind Default Descriptor Set
    if (vkDefaultDescriptorSet != VK_NULL_HANDLE && vkPipelineLayout != VK_NULL_HANDLE) {
        vkCmdBindDescriptorSets(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, 1, &vkDefaultDescriptorSet, 0, NULL);
    } else {
        TRACELOG(LOG_WARNING, "RLVK: Default descriptor set or pipeline layout not available for binding.");
    }
}


// Helper function to create a shader module from SPIR-V code
static VkShaderModule rlvkCreateShaderModule(const uint32_t* code, size_t codeSize) {
    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeSize;
    createInfo.pCode = code;

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(vkDevice, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        TRACELOG(LOG_ERROR, "RLVK: Failed to create shader module (size: %zu bytes)", codeSize);
        return VK_NULL_HANDLE;
    }
    TRACELOG(LOG_INFO, "RLVK: Shader module created successfully (size: %zu bytes)", codeSize);
    return shaderModule;
}

// Helper function to record and submit a one-time command buffer
static void rlvkRecordAndSubmitCommandBuffer(void (*record_commands)(VkCommandBuffer), const char* purpose_log_msg) {
    if (vkDevice == VK_NULL_HANDLE || vkCommandPool == VK_NULL_HANDLE || vkGraphicsQueue == VK_NULL_HANDLE) {
        TRACELOG(LOG_ERROR, "RLVK: Cannot execute one-time submit: Vulkan core components not ready.");
        return;
    }

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vkCommandPool; // Use the existing graphics command pool
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VkResult result = vkAllocateCommandBuffers(vkDevice, &allocInfo, &commandBuffer);
    if (result != VK_SUCCESS) {
        TRACELOG(LOG_ERROR, "RLVK: Failed to allocate command buffer for %s (Error: %i)", purpose_log_msg, result);
        return;
    }

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (result != VK_SUCCESS) {
        TRACELOG(LOG_ERROR, "RLVK: Failed to begin command buffer for %s (Error: %i)", purpose_log_msg, result);
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &commandBuffer);
        return;
    }

    record_commands(commandBuffer); // Call the provided function to record commands

    result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS) {
        TRACELOG(LOG_ERROR, "RLVK: Failed to end command buffer for %s (Error: %i)", purpose_log_msg, result);
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &commandBuffer);
        return;
    }

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Using a fence to wait for completion, though vkQueueWaitIdle is simpler for a single submission.
    // For robustness, a fence is better if other submissions could be happening.
    // Given this is a one-time setup operation, vkQueueWaitIdle is acceptable.
    result = vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        TRACELOG(LOG_ERROR, "RLVK: Failed to submit command buffer for %s (Error: %i)", purpose_log_msg, result);
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &commandBuffer);
        return;
    }

    result = vkQueueWaitIdle(vkGraphicsQueue); // Wait for the commands to complete
    if (result != VK_SUCCESS) {
        TRACELOG(LOG_ERROR, "RLVK: Failed to wait for queue idle after %s (Error: %i)", purpose_log_msg, result);
    }

    vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &commandBuffer);
    TRACELOG(LOG_DEBUG, "RLVK: Successfully executed one-time command buffer for %s.", purpose_log_msg);
}


// Commands for default texture layout transition and copy
static void recordDefaultTextureInitCommands(VkCommandBuffer commandBuffer) {
    // 1. Transition layout from UNDEFINED to TRANSFER_DST_OPTIMAL
    VkImageMemoryBarrier barrier = {0};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = vkDefaultTextureImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );

    // 2. Copy from staging buffer to image (assuming stagingBuffer is globally accessible or passed)
    // This requires stagingBuffer to be valid and filled at this point.
    // For simplicity, let's assume stagingBuffer is accessible. A better design would pass it.
    // We need to find where stagingBuffer is declared for default texture. It's local to rlvkInit.
    // This implies the copy needs to be part of the same command buffer recording as the transitions,
    // or stagingBuffer needs to be made accessible here.
    // For now, this function will only handle transitions. Copy will be inline or in another helper.
    // The task description implies this function should handle the copy.
    // Let's assume we pass stagingBuffer.buffer to this function, or make it static for a moment (not ideal).
    // For the subtask, I'll adjust rlvkInit to call this helper with necessary parameters, or inline this.
    // For now, this will be a placeholder for the copy part. The actual copy will be done in rlvkInit's command recording.

    // 3. Transition layout from TRANSFER_DST_OPTIMAL to SHADER_READ_ONLY_OPTIMAL
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );
}


void rlvkEndDrawing(void) {
    if (!rlvkReady) return;

    VkCommandBuffer currentCmdBuffer = vkCommandBuffers[acquiredImageIndex]; // Or vkCommandBuffers[currentFrame]

    // Upload vertex data to GPU buffer
    if (rlvkCPUVertexCount > 0 && rlvkGPUVertexBuffers != NULL) {
        rlvkBuffer currentGPUBuffer = rlvkGPUVertexBuffers[acquiredImageIndex]; // Or use currentFrame
        VkDeviceSize requiredSize = rlvkCPUVertexCount * sizeof(rlvkVertex);
        VkDeviceSize numVerticesToCopy = rlvkCPUVertexCount;

        if (requiredSize > rlvkGPUVertexBufferSize) {
            TRACELOG(LOG_WARNING, "RLVK: CPU vertex data size (%u bytes) exceeds GPU buffer capacity (%lu bytes). Clipping data.", (unsigned int)requiredSize, rlvkGPUVertexBufferSize);
            numVerticesToCopy = rlvkGPUVertexBufferSize / sizeof(rlvkVertex);
            requiredSize = numVerticesToCopy * sizeof(rlvkVertex);
        }

        if (requiredSize > 0) // Ensure there's actually something to copy after potential clipping
        {
            void* data;
            VkResult mapResult = vkMapMemory(vkDevice, currentGPUBuffer.memory, 0, requiredSize, 0, &data);
            if (mapResult == VK_SUCCESS) {
                memcpy(data, rlvkCPUVertexBuffer, requiredSize);
                vkUnmapMemory(vkDevice, currentGPUBuffer.memory);

                // Calculate MVP matrix from RLGL.State
                Matrix modelMatrix = RLGL.State.transformRequired ? RLGL.State.transform : rlMatrixIdentity();
                Matrix viewMatrix = RLGL.State.modelview;
                Matrix projectionMatrix = RLGL.State.projection;

                Matrix mvMatrix = rlMatrixMultiply(viewMatrix, modelMatrix);
                Matrix mvpMatrix = rlMatrixMultiply(projectionMatrix, mvMatrix);

                // Convert Matrix to float[16] for Vulkan
                // rlMatrixToFloat is a static function in rlgl.h implementation.
                // We need to ensure it's callable or replicate. For now, assume it's available or will be handled.
                // If it's not directly callable, we'll need to use its definition:
                float mvpFloats[16] = {
                    mvpMatrix.m0, mvpMatrix.m1, mvpMatrix.m2, mvpMatrix.m3,
                    mvpMatrix.m4, mvpMatrix.m5, mvpMatrix.m6, mvpMatrix.m7,
                    mvpMatrix.m8, mvpMatrix.m9, mvpMatrix.m10, mvpMatrix.m11,
                    mvpMatrix.m12, mvpMatrix.m13, mvpMatrix.m14, mvpMatrix.m15
                };

                // Upload MVP matrix via push constants
                // Ensure vkPipelineLayout was created with a push constant range for vertex shader
                if (vkPipelineLayout != VK_NULL_HANDLE) {
                    vkCmdPushConstants(
                        currentCmdBuffer,
                        vkPipelineLayout,
                        VK_SHADER_STAGE_VERTEX_BIT,
                        0,                          // offset
                        sizeof(float[16]),          // size (must match shader)
                        mvpFloats                   // pValues
                    );
                } else {
                    TRACELOG(LOG_WARNING, "RLVK: Pipeline layout is NULL, cannot push MVP constants.");
                }

                // Draw the vertices
                vkCmdDraw(currentCmdBuffer, numVerticesToCopy, 1, 0, 0);

            } else {
                TRACELOG(LOG_ERROR, "RLVK: Failed to map GPU vertex buffer memory (Error: %i)", mapResult);
            }
        }
    }

    vkCmdEndRenderPass(currentCmdBuffer);
    if (vkEndCommandBuffer(currentCmdBuffer) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to end command buffer.");
        return;
    }

    // Submit Command Buffer
    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {vkImageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &currentCmdBuffer;
    VkSemaphore signalSemaphores[] = {vkRenderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkResult submitResult = vkQueueSubmit(vkGraphicsQueue, 1, &submitInfo, vkInFlightFences[currentFrame]);
    if (submitResult != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to submit draw command buffer (Error: %i)", submitResult);
        return;
    }

    // Present Image
    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapchains[] = {vkSwapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &acquiredImageIndex;

    VkResult presentResult = vkQueuePresentKHR(vkPresentQueue, &presentInfo);

    if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) {
        TRACELOG(LOG_WARNING, "RLVK: Swapchain out of date or suboptimal during vkQueuePresentKHR. TODO: Recreate swapchain.");
        // rlRecreateSwapchain();
    } else if (presentResult != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "RLVK: Failed to present swapchain image (Error: %i)", presentResult);
    }

    // MAX_FRAMES_IN_FLIGHT needs to be well defined. Assuming it's vkSwapchainImageCount for now for simplicity of fence management.
    // If MAX_FRAMES_IN_FLIGHT is less than vkSwapchainImageCount, the logic for fences and command buffers needs adjustment.
    currentFrame = (currentFrame + 1) % vkSwapchainImageCount;
}

void rlvkClearBackground(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    currentClearColor.float32[0] = (float)r / 255.0f;
    currentClearColor.float32[1] = (float)g / 255.0f;
    currentClearColor.float32[2] = (float)b / 255.0f;
    currentClearColor.float32[3] = (float)a / 255.0f;
}

unsigned int rlvkLoadTexture(const void *data, int width, int height, int format, int mipmaps) {
    printf("rlvkLoadTexture called (STUB)\n");
    return 0; // Return a dummy ID
}

void rlvkUnloadTexture(unsigned int id) {
    printf("rlvkUnloadTexture called (STUB) for ID: %u\n", id);
}

unsigned int rlvkLoadShaderCode(const char *vsCode, const char *fsCode) {
    printf("rlvkLoadShaderCode called (STUB)\n");
    return 0; // Return a dummy ID
}

void rlvkUnloadShaderProgram(unsigned int id) {
    printf("rlvkUnloadShaderProgram called (STUB) for ID: %u\n", id);
}

int rlvkGetLocationUniform(unsigned int shaderId, const char *uniformName) {
    // printf("rlvkGetLocationUniform called (STUB) for shader ID: %u, uniform: %s\n", shaderId, uniformName);
    return -1;
}

int rlvkGetLocationAttrib(unsigned int shaderId, const char *attribName) {
    // printf("rlvkGetLocationAttrib called (STUB) for shader ID: %u, attrib: %s\n", shaderId, attribName);
    return -1;
}

void rlvkSetUniform(int locIndex, const void *value, int uniformType, int count) {
    // printf("rlvkSetUniform called (STUB) for locIndex: %d\n", locIndex);
}

void rlvkSetPrimitiveMode(int mode) {
    // This function is used to store the primitive mode set by rlBegin.
    // It will be used later when creating/binding graphics pipelines.
    // For now, Vulkan drawing defaults to triangles.
    // RL_QUADS are typically handled by sending 4 vertices and drawing them as two triangles.
    // RL_LINES will require a different pipeline state.
    currentPrimitiveMode = mode;
    // TRACELOG(LOG_DEBUG, "RLVK: Primitive mode set to %d", mode);
}

// ... other rlgl equivalent function stub implementations ...
// TRACELOG can be used for more detailed stub logging if utils.h is appropriately included and linked.
// For now, simple printf might be fine for basic stub verification.
