#include "rlvk.h"
#include "utils.h"      // For TRACELOG
#include <stdio.h>      // For TRACELOG / printf
#include <stdlib.h>     // For RL_MALLOC, RL_FREE, NULL
#include <string.h>     // For strcmp, memset
#include <stdbool.h>    // For bool type

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

    rlvkReady = true; // All initialization steps completed successfully
    TRACELOG(LOG_INFO, "RLVK: Vulkan backend initialized successfully.");
}

void rlvkClose(void) {
    TRACELOG(LOG_INFO, "RLVK: Closing Vulkan backend.");

    if (vkDevice != VK_NULL_HANDLE) {
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

void rlvkBeginDrawing(void) {
    if (!rlvkReady) return;

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
}

void rlvkEndDrawing(void) {
    if (!rlvkReady) return;

    VkCommandBuffer currentCmdBuffer = vkCommandBuffers[acquiredImageIndex]; // Or vkCommandBuffers[currentFrame]

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

// ... other rlgl equivalent function stub implementations ...
// TRACELOG can be used for more detailed stub logging if utils.h is appropriately included and linked.
// For now, simple printf might be fine for basic stub verification.
