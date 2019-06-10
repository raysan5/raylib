#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/vulkan.h>

#ifndef GLAD_IMPL_UTIL_C_
#define GLAD_IMPL_UTIL_C_

#ifdef _MSC_VER
#define GLAD_IMPL_UTIL_SSCANF sscanf_s
#else
#define GLAD_IMPL_UTIL_SSCANF sscanf
#endif

#endif /* GLAD_IMPL_UTIL_C_ */


int GLAD_VK_VERSION_1_0 = 0;
int GLAD_VK_VERSION_1_1 = 0;
int GLAD_VK_EXT_debug_report = 0;
int GLAD_VK_KHR_surface = 0;
int GLAD_VK_KHR_swapchain = 0;



PFN_vkAcquireNextImage2KHR glad_vkAcquireNextImage2KHR = NULL;
PFN_vkAcquireNextImageKHR glad_vkAcquireNextImageKHR = NULL;
PFN_vkAllocateCommandBuffers glad_vkAllocateCommandBuffers = NULL;
PFN_vkAllocateDescriptorSets glad_vkAllocateDescriptorSets = NULL;
PFN_vkAllocateMemory glad_vkAllocateMemory = NULL;
PFN_vkBeginCommandBuffer glad_vkBeginCommandBuffer = NULL;
PFN_vkBindBufferMemory glad_vkBindBufferMemory = NULL;
PFN_vkBindBufferMemory2 glad_vkBindBufferMemory2 = NULL;
PFN_vkBindImageMemory glad_vkBindImageMemory = NULL;
PFN_vkBindImageMemory2 glad_vkBindImageMemory2 = NULL;
PFN_vkCmdBeginQuery glad_vkCmdBeginQuery = NULL;
PFN_vkCmdBeginRenderPass glad_vkCmdBeginRenderPass = NULL;
PFN_vkCmdBindDescriptorSets glad_vkCmdBindDescriptorSets = NULL;
PFN_vkCmdBindIndexBuffer glad_vkCmdBindIndexBuffer = NULL;
PFN_vkCmdBindPipeline glad_vkCmdBindPipeline = NULL;
PFN_vkCmdBindVertexBuffers glad_vkCmdBindVertexBuffers = NULL;
PFN_vkCmdBlitImage glad_vkCmdBlitImage = NULL;
PFN_vkCmdClearAttachments glad_vkCmdClearAttachments = NULL;
PFN_vkCmdClearColorImage glad_vkCmdClearColorImage = NULL;
PFN_vkCmdClearDepthStencilImage glad_vkCmdClearDepthStencilImage = NULL;
PFN_vkCmdCopyBuffer glad_vkCmdCopyBuffer = NULL;
PFN_vkCmdCopyBufferToImage glad_vkCmdCopyBufferToImage = NULL;
PFN_vkCmdCopyImage glad_vkCmdCopyImage = NULL;
PFN_vkCmdCopyImageToBuffer glad_vkCmdCopyImageToBuffer = NULL;
PFN_vkCmdCopyQueryPoolResults glad_vkCmdCopyQueryPoolResults = NULL;
PFN_vkCmdDispatch glad_vkCmdDispatch = NULL;
PFN_vkCmdDispatchBase glad_vkCmdDispatchBase = NULL;
PFN_vkCmdDispatchIndirect glad_vkCmdDispatchIndirect = NULL;
PFN_vkCmdDraw glad_vkCmdDraw = NULL;
PFN_vkCmdDrawIndexed glad_vkCmdDrawIndexed = NULL;
PFN_vkCmdDrawIndexedIndirect glad_vkCmdDrawIndexedIndirect = NULL;
PFN_vkCmdDrawIndirect glad_vkCmdDrawIndirect = NULL;
PFN_vkCmdEndQuery glad_vkCmdEndQuery = NULL;
PFN_vkCmdEndRenderPass glad_vkCmdEndRenderPass = NULL;
PFN_vkCmdExecuteCommands glad_vkCmdExecuteCommands = NULL;
PFN_vkCmdFillBuffer glad_vkCmdFillBuffer = NULL;
PFN_vkCmdNextSubpass glad_vkCmdNextSubpass = NULL;
PFN_vkCmdPipelineBarrier glad_vkCmdPipelineBarrier = NULL;
PFN_vkCmdPushConstants glad_vkCmdPushConstants = NULL;
PFN_vkCmdResetEvent glad_vkCmdResetEvent = NULL;
PFN_vkCmdResetQueryPool glad_vkCmdResetQueryPool = NULL;
PFN_vkCmdResolveImage glad_vkCmdResolveImage = NULL;
PFN_vkCmdSetBlendConstants glad_vkCmdSetBlendConstants = NULL;
PFN_vkCmdSetDepthBias glad_vkCmdSetDepthBias = NULL;
PFN_vkCmdSetDepthBounds glad_vkCmdSetDepthBounds = NULL;
PFN_vkCmdSetDeviceMask glad_vkCmdSetDeviceMask = NULL;
PFN_vkCmdSetEvent glad_vkCmdSetEvent = NULL;
PFN_vkCmdSetLineWidth glad_vkCmdSetLineWidth = NULL;
PFN_vkCmdSetScissor glad_vkCmdSetScissor = NULL;
PFN_vkCmdSetStencilCompareMask glad_vkCmdSetStencilCompareMask = NULL;
PFN_vkCmdSetStencilReference glad_vkCmdSetStencilReference = NULL;
PFN_vkCmdSetStencilWriteMask glad_vkCmdSetStencilWriteMask = NULL;
PFN_vkCmdSetViewport glad_vkCmdSetViewport = NULL;
PFN_vkCmdUpdateBuffer glad_vkCmdUpdateBuffer = NULL;
PFN_vkCmdWaitEvents glad_vkCmdWaitEvents = NULL;
PFN_vkCmdWriteTimestamp glad_vkCmdWriteTimestamp = NULL;
PFN_vkCreateBuffer glad_vkCreateBuffer = NULL;
PFN_vkCreateBufferView glad_vkCreateBufferView = NULL;
PFN_vkCreateCommandPool glad_vkCreateCommandPool = NULL;
PFN_vkCreateComputePipelines glad_vkCreateComputePipelines = NULL;
PFN_vkCreateDebugReportCallbackEXT glad_vkCreateDebugReportCallbackEXT = NULL;
PFN_vkCreateDescriptorPool glad_vkCreateDescriptorPool = NULL;
PFN_vkCreateDescriptorSetLayout glad_vkCreateDescriptorSetLayout = NULL;
PFN_vkCreateDescriptorUpdateTemplate glad_vkCreateDescriptorUpdateTemplate = NULL;
PFN_vkCreateDevice glad_vkCreateDevice = NULL;
PFN_vkCreateEvent glad_vkCreateEvent = NULL;
PFN_vkCreateFence glad_vkCreateFence = NULL;
PFN_vkCreateFramebuffer glad_vkCreateFramebuffer = NULL;
PFN_vkCreateGraphicsPipelines glad_vkCreateGraphicsPipelines = NULL;
PFN_vkCreateImage glad_vkCreateImage = NULL;
PFN_vkCreateImageView glad_vkCreateImageView = NULL;
PFN_vkCreateInstance glad_vkCreateInstance = NULL;
PFN_vkCreatePipelineCache glad_vkCreatePipelineCache = NULL;
PFN_vkCreatePipelineLayout glad_vkCreatePipelineLayout = NULL;
PFN_vkCreateQueryPool glad_vkCreateQueryPool = NULL;
PFN_vkCreateRenderPass glad_vkCreateRenderPass = NULL;
PFN_vkCreateSampler glad_vkCreateSampler = NULL;
PFN_vkCreateSamplerYcbcrConversion glad_vkCreateSamplerYcbcrConversion = NULL;
PFN_vkCreateSemaphore glad_vkCreateSemaphore = NULL;
PFN_vkCreateShaderModule glad_vkCreateShaderModule = NULL;
PFN_vkCreateSwapchainKHR glad_vkCreateSwapchainKHR = NULL;
PFN_vkDebugReportMessageEXT glad_vkDebugReportMessageEXT = NULL;
PFN_vkDestroyBuffer glad_vkDestroyBuffer = NULL;
PFN_vkDestroyBufferView glad_vkDestroyBufferView = NULL;
PFN_vkDestroyCommandPool glad_vkDestroyCommandPool = NULL;
PFN_vkDestroyDebugReportCallbackEXT glad_vkDestroyDebugReportCallbackEXT = NULL;
PFN_vkDestroyDescriptorPool glad_vkDestroyDescriptorPool = NULL;
PFN_vkDestroyDescriptorSetLayout glad_vkDestroyDescriptorSetLayout = NULL;
PFN_vkDestroyDescriptorUpdateTemplate glad_vkDestroyDescriptorUpdateTemplate = NULL;
PFN_vkDestroyDevice glad_vkDestroyDevice = NULL;
PFN_vkDestroyEvent glad_vkDestroyEvent = NULL;
PFN_vkDestroyFence glad_vkDestroyFence = NULL;
PFN_vkDestroyFramebuffer glad_vkDestroyFramebuffer = NULL;
PFN_vkDestroyImage glad_vkDestroyImage = NULL;
PFN_vkDestroyImageView glad_vkDestroyImageView = NULL;
PFN_vkDestroyInstance glad_vkDestroyInstance = NULL;
PFN_vkDestroyPipeline glad_vkDestroyPipeline = NULL;
PFN_vkDestroyPipelineCache glad_vkDestroyPipelineCache = NULL;
PFN_vkDestroyPipelineLayout glad_vkDestroyPipelineLayout = NULL;
PFN_vkDestroyQueryPool glad_vkDestroyQueryPool = NULL;
PFN_vkDestroyRenderPass glad_vkDestroyRenderPass = NULL;
PFN_vkDestroySampler glad_vkDestroySampler = NULL;
PFN_vkDestroySamplerYcbcrConversion glad_vkDestroySamplerYcbcrConversion = NULL;
PFN_vkDestroySemaphore glad_vkDestroySemaphore = NULL;
PFN_vkDestroyShaderModule glad_vkDestroyShaderModule = NULL;
PFN_vkDestroySurfaceKHR glad_vkDestroySurfaceKHR = NULL;
PFN_vkDestroySwapchainKHR glad_vkDestroySwapchainKHR = NULL;
PFN_vkDeviceWaitIdle glad_vkDeviceWaitIdle = NULL;
PFN_vkEndCommandBuffer glad_vkEndCommandBuffer = NULL;
PFN_vkEnumerateDeviceExtensionProperties glad_vkEnumerateDeviceExtensionProperties = NULL;
PFN_vkEnumerateDeviceLayerProperties glad_vkEnumerateDeviceLayerProperties = NULL;
PFN_vkEnumerateInstanceExtensionProperties glad_vkEnumerateInstanceExtensionProperties = NULL;
PFN_vkEnumerateInstanceLayerProperties glad_vkEnumerateInstanceLayerProperties = NULL;
PFN_vkEnumerateInstanceVersion glad_vkEnumerateInstanceVersion = NULL;
PFN_vkEnumeratePhysicalDeviceGroups glad_vkEnumeratePhysicalDeviceGroups = NULL;
PFN_vkEnumeratePhysicalDevices glad_vkEnumeratePhysicalDevices = NULL;
PFN_vkFlushMappedMemoryRanges glad_vkFlushMappedMemoryRanges = NULL;
PFN_vkFreeCommandBuffers glad_vkFreeCommandBuffers = NULL;
PFN_vkFreeDescriptorSets glad_vkFreeDescriptorSets = NULL;
PFN_vkFreeMemory glad_vkFreeMemory = NULL;
PFN_vkGetBufferMemoryRequirements glad_vkGetBufferMemoryRequirements = NULL;
PFN_vkGetBufferMemoryRequirements2 glad_vkGetBufferMemoryRequirements2 = NULL;
PFN_vkGetDescriptorSetLayoutSupport glad_vkGetDescriptorSetLayoutSupport = NULL;
PFN_vkGetDeviceGroupPeerMemoryFeatures glad_vkGetDeviceGroupPeerMemoryFeatures = NULL;
PFN_vkGetDeviceGroupPresentCapabilitiesKHR glad_vkGetDeviceGroupPresentCapabilitiesKHR = NULL;
PFN_vkGetDeviceGroupSurfacePresentModesKHR glad_vkGetDeviceGroupSurfacePresentModesKHR = NULL;
PFN_vkGetDeviceMemoryCommitment glad_vkGetDeviceMemoryCommitment = NULL;
PFN_vkGetDeviceProcAddr glad_vkGetDeviceProcAddr = NULL;
PFN_vkGetDeviceQueue glad_vkGetDeviceQueue = NULL;
PFN_vkGetDeviceQueue2 glad_vkGetDeviceQueue2 = NULL;
PFN_vkGetEventStatus glad_vkGetEventStatus = NULL;
PFN_vkGetFenceStatus glad_vkGetFenceStatus = NULL;
PFN_vkGetImageMemoryRequirements glad_vkGetImageMemoryRequirements = NULL;
PFN_vkGetImageMemoryRequirements2 glad_vkGetImageMemoryRequirements2 = NULL;
PFN_vkGetImageSparseMemoryRequirements glad_vkGetImageSparseMemoryRequirements = NULL;
PFN_vkGetImageSparseMemoryRequirements2 glad_vkGetImageSparseMemoryRequirements2 = NULL;
PFN_vkGetImageSubresourceLayout glad_vkGetImageSubresourceLayout = NULL;
PFN_vkGetInstanceProcAddr glad_vkGetInstanceProcAddr = NULL;
PFN_vkGetPhysicalDeviceExternalBufferProperties glad_vkGetPhysicalDeviceExternalBufferProperties = NULL;
PFN_vkGetPhysicalDeviceExternalFenceProperties glad_vkGetPhysicalDeviceExternalFenceProperties = NULL;
PFN_vkGetPhysicalDeviceExternalSemaphoreProperties glad_vkGetPhysicalDeviceExternalSemaphoreProperties = NULL;
PFN_vkGetPhysicalDeviceFeatures glad_vkGetPhysicalDeviceFeatures = NULL;
PFN_vkGetPhysicalDeviceFeatures2 glad_vkGetPhysicalDeviceFeatures2 = NULL;
PFN_vkGetPhysicalDeviceFormatProperties glad_vkGetPhysicalDeviceFormatProperties = NULL;
PFN_vkGetPhysicalDeviceFormatProperties2 glad_vkGetPhysicalDeviceFormatProperties2 = NULL;
PFN_vkGetPhysicalDeviceImageFormatProperties glad_vkGetPhysicalDeviceImageFormatProperties = NULL;
PFN_vkGetPhysicalDeviceImageFormatProperties2 glad_vkGetPhysicalDeviceImageFormatProperties2 = NULL;
PFN_vkGetPhysicalDeviceMemoryProperties glad_vkGetPhysicalDeviceMemoryProperties = NULL;
PFN_vkGetPhysicalDeviceMemoryProperties2 glad_vkGetPhysicalDeviceMemoryProperties2 = NULL;
PFN_vkGetPhysicalDevicePresentRectanglesKHR glad_vkGetPhysicalDevicePresentRectanglesKHR = NULL;
PFN_vkGetPhysicalDeviceProperties glad_vkGetPhysicalDeviceProperties = NULL;
PFN_vkGetPhysicalDeviceProperties2 glad_vkGetPhysicalDeviceProperties2 = NULL;
PFN_vkGetPhysicalDeviceQueueFamilyProperties glad_vkGetPhysicalDeviceQueueFamilyProperties = NULL;
PFN_vkGetPhysicalDeviceQueueFamilyProperties2 glad_vkGetPhysicalDeviceQueueFamilyProperties2 = NULL;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties glad_vkGetPhysicalDeviceSparseImageFormatProperties = NULL;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 glad_vkGetPhysicalDeviceSparseImageFormatProperties2 = NULL;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR glad_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR glad_vkGetPhysicalDeviceSurfaceFormatsKHR = NULL;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR glad_vkGetPhysicalDeviceSurfacePresentModesKHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR glad_vkGetPhysicalDeviceSurfaceSupportKHR = NULL;
PFN_vkGetPipelineCacheData glad_vkGetPipelineCacheData = NULL;
PFN_vkGetQueryPoolResults glad_vkGetQueryPoolResults = NULL;
PFN_vkGetRenderAreaGranularity glad_vkGetRenderAreaGranularity = NULL;
PFN_vkGetSwapchainImagesKHR glad_vkGetSwapchainImagesKHR = NULL;
PFN_vkInvalidateMappedMemoryRanges glad_vkInvalidateMappedMemoryRanges = NULL;
PFN_vkMapMemory glad_vkMapMemory = NULL;
PFN_vkMergePipelineCaches glad_vkMergePipelineCaches = NULL;
PFN_vkQueueBindSparse glad_vkQueueBindSparse = NULL;
PFN_vkQueuePresentKHR glad_vkQueuePresentKHR = NULL;
PFN_vkQueueSubmit glad_vkQueueSubmit = NULL;
PFN_vkQueueWaitIdle glad_vkQueueWaitIdle = NULL;
PFN_vkResetCommandBuffer glad_vkResetCommandBuffer = NULL;
PFN_vkResetCommandPool glad_vkResetCommandPool = NULL;
PFN_vkResetDescriptorPool glad_vkResetDescriptorPool = NULL;
PFN_vkResetEvent glad_vkResetEvent = NULL;
PFN_vkResetFences glad_vkResetFences = NULL;
PFN_vkSetEvent glad_vkSetEvent = NULL;
PFN_vkTrimCommandPool glad_vkTrimCommandPool = NULL;
PFN_vkUnmapMemory glad_vkUnmapMemory = NULL;
PFN_vkUpdateDescriptorSetWithTemplate glad_vkUpdateDescriptorSetWithTemplate = NULL;
PFN_vkUpdateDescriptorSets glad_vkUpdateDescriptorSets = NULL;
PFN_vkWaitForFences glad_vkWaitForFences = NULL;


static void glad_vk_load_VK_VERSION_1_0( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_VERSION_1_0) return;
    vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers) load("vkAllocateCommandBuffers", userptr);
    vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets) load("vkAllocateDescriptorSets", userptr);
    vkAllocateMemory = (PFN_vkAllocateMemory) load("vkAllocateMemory", userptr);
    vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer) load("vkBeginCommandBuffer", userptr);
    vkBindBufferMemory = (PFN_vkBindBufferMemory) load("vkBindBufferMemory", userptr);
    vkBindImageMemory = (PFN_vkBindImageMemory) load("vkBindImageMemory", userptr);
    vkCmdBeginQuery = (PFN_vkCmdBeginQuery) load("vkCmdBeginQuery", userptr);
    vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass) load("vkCmdBeginRenderPass", userptr);
    vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets) load("vkCmdBindDescriptorSets", userptr);
    vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer) load("vkCmdBindIndexBuffer", userptr);
    vkCmdBindPipeline = (PFN_vkCmdBindPipeline) load("vkCmdBindPipeline", userptr);
    vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers) load("vkCmdBindVertexBuffers", userptr);
    vkCmdBlitImage = (PFN_vkCmdBlitImage) load("vkCmdBlitImage", userptr);
    vkCmdClearAttachments = (PFN_vkCmdClearAttachments) load("vkCmdClearAttachments", userptr);
    vkCmdClearColorImage = (PFN_vkCmdClearColorImage) load("vkCmdClearColorImage", userptr);
    vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage) load("vkCmdClearDepthStencilImage", userptr);
    vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer) load("vkCmdCopyBuffer", userptr);
    vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage) load("vkCmdCopyBufferToImage", userptr);
    vkCmdCopyImage = (PFN_vkCmdCopyImage) load("vkCmdCopyImage", userptr);
    vkCmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer) load("vkCmdCopyImageToBuffer", userptr);
    vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults) load("vkCmdCopyQueryPoolResults", userptr);
    vkCmdDispatch = (PFN_vkCmdDispatch) load("vkCmdDispatch", userptr);
    vkCmdDispatchIndirect = (PFN_vkCmdDispatchIndirect) load("vkCmdDispatchIndirect", userptr);
    vkCmdDraw = (PFN_vkCmdDraw) load("vkCmdDraw", userptr);
    vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed) load("vkCmdDrawIndexed", userptr);
    vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect) load("vkCmdDrawIndexedIndirect", userptr);
    vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect) load("vkCmdDrawIndirect", userptr);
    vkCmdEndQuery = (PFN_vkCmdEndQuery) load("vkCmdEndQuery", userptr);
    vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass) load("vkCmdEndRenderPass", userptr);
    vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands) load("vkCmdExecuteCommands", userptr);
    vkCmdFillBuffer = (PFN_vkCmdFillBuffer) load("vkCmdFillBuffer", userptr);
    vkCmdNextSubpass = (PFN_vkCmdNextSubpass) load("vkCmdNextSubpass", userptr);
    vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier) load("vkCmdPipelineBarrier", userptr);
    vkCmdPushConstants = (PFN_vkCmdPushConstants) load("vkCmdPushConstants", userptr);
    vkCmdResetEvent = (PFN_vkCmdResetEvent) load("vkCmdResetEvent", userptr);
    vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool) load("vkCmdResetQueryPool", userptr);
    vkCmdResolveImage = (PFN_vkCmdResolveImage) load("vkCmdResolveImage", userptr);
    vkCmdSetBlendConstants = (PFN_vkCmdSetBlendConstants) load("vkCmdSetBlendConstants", userptr);
    vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias) load("vkCmdSetDepthBias", userptr);
    vkCmdSetDepthBounds = (PFN_vkCmdSetDepthBounds) load("vkCmdSetDepthBounds", userptr);
    vkCmdSetEvent = (PFN_vkCmdSetEvent) load("vkCmdSetEvent", userptr);
    vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth) load("vkCmdSetLineWidth", userptr);
    vkCmdSetScissor = (PFN_vkCmdSetScissor) load("vkCmdSetScissor", userptr);
    vkCmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask) load("vkCmdSetStencilCompareMask", userptr);
    vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference) load("vkCmdSetStencilReference", userptr);
    vkCmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask) load("vkCmdSetStencilWriteMask", userptr);
    vkCmdSetViewport = (PFN_vkCmdSetViewport) load("vkCmdSetViewport", userptr);
    vkCmdUpdateBuffer = (PFN_vkCmdUpdateBuffer) load("vkCmdUpdateBuffer", userptr);
    vkCmdWaitEvents = (PFN_vkCmdWaitEvents) load("vkCmdWaitEvents", userptr);
    vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp) load("vkCmdWriteTimestamp", userptr);
    vkCreateBuffer = (PFN_vkCreateBuffer) load("vkCreateBuffer", userptr);
    vkCreateBufferView = (PFN_vkCreateBufferView) load("vkCreateBufferView", userptr);
    vkCreateCommandPool = (PFN_vkCreateCommandPool) load("vkCreateCommandPool", userptr);
    vkCreateComputePipelines = (PFN_vkCreateComputePipelines) load("vkCreateComputePipelines", userptr);
    vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool) load("vkCreateDescriptorPool", userptr);
    vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout) load("vkCreateDescriptorSetLayout", userptr);
    vkCreateDevice = (PFN_vkCreateDevice) load("vkCreateDevice", userptr);
    vkCreateEvent = (PFN_vkCreateEvent) load("vkCreateEvent", userptr);
    vkCreateFence = (PFN_vkCreateFence) load("vkCreateFence", userptr);
    vkCreateFramebuffer = (PFN_vkCreateFramebuffer) load("vkCreateFramebuffer", userptr);
    vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines) load("vkCreateGraphicsPipelines", userptr);
    vkCreateImage = (PFN_vkCreateImage) load("vkCreateImage", userptr);
    vkCreateImageView = (PFN_vkCreateImageView) load("vkCreateImageView", userptr);
    vkCreateInstance = (PFN_vkCreateInstance) load("vkCreateInstance", userptr);
    vkCreatePipelineCache = (PFN_vkCreatePipelineCache) load("vkCreatePipelineCache", userptr);
    vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout) load("vkCreatePipelineLayout", userptr);
    vkCreateQueryPool = (PFN_vkCreateQueryPool) load("vkCreateQueryPool", userptr);
    vkCreateRenderPass = (PFN_vkCreateRenderPass) load("vkCreateRenderPass", userptr);
    vkCreateSampler = (PFN_vkCreateSampler) load("vkCreateSampler", userptr);
    vkCreateSemaphore = (PFN_vkCreateSemaphore) load("vkCreateSemaphore", userptr);
    vkCreateShaderModule = (PFN_vkCreateShaderModule) load("vkCreateShaderModule", userptr);
    vkDestroyBuffer = (PFN_vkDestroyBuffer) load("vkDestroyBuffer", userptr);
    vkDestroyBufferView = (PFN_vkDestroyBufferView) load("vkDestroyBufferView", userptr);
    vkDestroyCommandPool = (PFN_vkDestroyCommandPool) load("vkDestroyCommandPool", userptr);
    vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool) load("vkDestroyDescriptorPool", userptr);
    vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout) load("vkDestroyDescriptorSetLayout", userptr);
    vkDestroyDevice = (PFN_vkDestroyDevice) load("vkDestroyDevice", userptr);
    vkDestroyEvent = (PFN_vkDestroyEvent) load("vkDestroyEvent", userptr);
    vkDestroyFence = (PFN_vkDestroyFence) load("vkDestroyFence", userptr);
    vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer) load("vkDestroyFramebuffer", userptr);
    vkDestroyImage = (PFN_vkDestroyImage) load("vkDestroyImage", userptr);
    vkDestroyImageView = (PFN_vkDestroyImageView) load("vkDestroyImageView", userptr);
    vkDestroyInstance = (PFN_vkDestroyInstance) load("vkDestroyInstance", userptr);
    vkDestroyPipeline = (PFN_vkDestroyPipeline) load("vkDestroyPipeline", userptr);
    vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache) load("vkDestroyPipelineCache", userptr);
    vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout) load("vkDestroyPipelineLayout", userptr);
    vkDestroyQueryPool = (PFN_vkDestroyQueryPool) load("vkDestroyQueryPool", userptr);
    vkDestroyRenderPass = (PFN_vkDestroyRenderPass) load("vkDestroyRenderPass", userptr);
    vkDestroySampler = (PFN_vkDestroySampler) load("vkDestroySampler", userptr);
    vkDestroySemaphore = (PFN_vkDestroySemaphore) load("vkDestroySemaphore", userptr);
    vkDestroyShaderModule = (PFN_vkDestroyShaderModule) load("vkDestroyShaderModule", userptr);
    vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle) load("vkDeviceWaitIdle", userptr);
    vkEndCommandBuffer = (PFN_vkEndCommandBuffer) load("vkEndCommandBuffer", userptr);
    vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) load("vkEnumerateDeviceExtensionProperties", userptr);
    vkEnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties) load("vkEnumerateDeviceLayerProperties", userptr);
    vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties) load("vkEnumerateInstanceExtensionProperties", userptr);
    vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties) load("vkEnumerateInstanceLayerProperties", userptr);
    vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) load("vkEnumeratePhysicalDevices", userptr);
    vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges) load("vkFlushMappedMemoryRanges", userptr);
    vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers) load("vkFreeCommandBuffers", userptr);
    vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets) load("vkFreeDescriptorSets", userptr);
    vkFreeMemory = (PFN_vkFreeMemory) load("vkFreeMemory", userptr);
    vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements) load("vkGetBufferMemoryRequirements", userptr);
    vkGetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment) load("vkGetDeviceMemoryCommitment", userptr);
    vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr) load("vkGetDeviceProcAddr", userptr);
    vkGetDeviceQueue = (PFN_vkGetDeviceQueue) load("vkGetDeviceQueue", userptr);
    vkGetEventStatus = (PFN_vkGetEventStatus) load("vkGetEventStatus", userptr);
    vkGetFenceStatus = (PFN_vkGetFenceStatus) load("vkGetFenceStatus", userptr);
    vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements) load("vkGetImageMemoryRequirements", userptr);
    vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements) load("vkGetImageSparseMemoryRequirements", userptr);
    vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout) load("vkGetImageSubresourceLayout", userptr);
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) load("vkGetInstanceProcAddr", userptr);
    vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) load("vkGetPhysicalDeviceFeatures", userptr);
    vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties) load("vkGetPhysicalDeviceFormatProperties", userptr);
    vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties) load("vkGetPhysicalDeviceImageFormatProperties", userptr);
    vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties) load("vkGetPhysicalDeviceMemoryProperties", userptr);
    vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) load("vkGetPhysicalDeviceProperties", userptr);
    vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) load("vkGetPhysicalDeviceQueueFamilyProperties", userptr);
    vkGetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties) load("vkGetPhysicalDeviceSparseImageFormatProperties", userptr);
    vkGetPipelineCacheData = (PFN_vkGetPipelineCacheData) load("vkGetPipelineCacheData", userptr);
    vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults) load("vkGetQueryPoolResults", userptr);
    vkGetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity) load("vkGetRenderAreaGranularity", userptr);
    vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges) load("vkInvalidateMappedMemoryRanges", userptr);
    vkMapMemory = (PFN_vkMapMemory) load("vkMapMemory", userptr);
    vkMergePipelineCaches = (PFN_vkMergePipelineCaches) load("vkMergePipelineCaches", userptr);
    vkQueueBindSparse = (PFN_vkQueueBindSparse) load("vkQueueBindSparse", userptr);
    vkQueueSubmit = (PFN_vkQueueSubmit) load("vkQueueSubmit", userptr);
    vkQueueWaitIdle = (PFN_vkQueueWaitIdle) load("vkQueueWaitIdle", userptr);
    vkResetCommandBuffer = (PFN_vkResetCommandBuffer) load("vkResetCommandBuffer", userptr);
    vkResetCommandPool = (PFN_vkResetCommandPool) load("vkResetCommandPool", userptr);
    vkResetDescriptorPool = (PFN_vkResetDescriptorPool) load("vkResetDescriptorPool", userptr);
    vkResetEvent = (PFN_vkResetEvent) load("vkResetEvent", userptr);
    vkResetFences = (PFN_vkResetFences) load("vkResetFences", userptr);
    vkSetEvent = (PFN_vkSetEvent) load("vkSetEvent", userptr);
    vkUnmapMemory = (PFN_vkUnmapMemory) load("vkUnmapMemory", userptr);
    vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets) load("vkUpdateDescriptorSets", userptr);
    vkWaitForFences = (PFN_vkWaitForFences) load("vkWaitForFences", userptr);
}
static void glad_vk_load_VK_VERSION_1_1( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_VERSION_1_1) return;
    vkBindBufferMemory2 = (PFN_vkBindBufferMemory2) load("vkBindBufferMemory2", userptr);
    vkBindImageMemory2 = (PFN_vkBindImageMemory2) load("vkBindImageMemory2", userptr);
    vkCmdDispatchBase = (PFN_vkCmdDispatchBase) load("vkCmdDispatchBase", userptr);
    vkCmdSetDeviceMask = (PFN_vkCmdSetDeviceMask) load("vkCmdSetDeviceMask", userptr);
    vkCreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate) load("vkCreateDescriptorUpdateTemplate", userptr);
    vkCreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion) load("vkCreateSamplerYcbcrConversion", userptr);
    vkDestroyDescriptorUpdateTemplate = (PFN_vkDestroyDescriptorUpdateTemplate) load("vkDestroyDescriptorUpdateTemplate", userptr);
    vkDestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion) load("vkDestroySamplerYcbcrConversion", userptr);
    vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion) load("vkEnumerateInstanceVersion", userptr);
    vkEnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups) load("vkEnumeratePhysicalDeviceGroups", userptr);
    vkGetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2) load("vkGetBufferMemoryRequirements2", userptr);
    vkGetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport) load("vkGetDescriptorSetLayoutSupport", userptr);
    vkGetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures) load("vkGetDeviceGroupPeerMemoryFeatures", userptr);
    vkGetDeviceQueue2 = (PFN_vkGetDeviceQueue2) load("vkGetDeviceQueue2", userptr);
    vkGetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2) load("vkGetImageMemoryRequirements2", userptr);
    vkGetImageSparseMemoryRequirements2 = (PFN_vkGetImageSparseMemoryRequirements2) load("vkGetImageSparseMemoryRequirements2", userptr);
    vkGetPhysicalDeviceExternalBufferProperties = (PFN_vkGetPhysicalDeviceExternalBufferProperties) load("vkGetPhysicalDeviceExternalBufferProperties", userptr);
    vkGetPhysicalDeviceExternalFenceProperties = (PFN_vkGetPhysicalDeviceExternalFenceProperties) load("vkGetPhysicalDeviceExternalFenceProperties", userptr);
    vkGetPhysicalDeviceExternalSemaphoreProperties = (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties) load("vkGetPhysicalDeviceExternalSemaphoreProperties", userptr);
    vkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2) load("vkGetPhysicalDeviceFeatures2", userptr);
    vkGetPhysicalDeviceFormatProperties2 = (PFN_vkGetPhysicalDeviceFormatProperties2) load("vkGetPhysicalDeviceFormatProperties2", userptr);
    vkGetPhysicalDeviceImageFormatProperties2 = (PFN_vkGetPhysicalDeviceImageFormatProperties2) load("vkGetPhysicalDeviceImageFormatProperties2", userptr);
    vkGetPhysicalDeviceMemoryProperties2 = (PFN_vkGetPhysicalDeviceMemoryProperties2) load("vkGetPhysicalDeviceMemoryProperties2", userptr);
    vkGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2) load("vkGetPhysicalDeviceProperties2", userptr);
    vkGetPhysicalDeviceQueueFamilyProperties2 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2) load("vkGetPhysicalDeviceQueueFamilyProperties2", userptr);
    vkGetPhysicalDeviceSparseImageFormatProperties2 = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2) load("vkGetPhysicalDeviceSparseImageFormatProperties2", userptr);
    vkTrimCommandPool = (PFN_vkTrimCommandPool) load("vkTrimCommandPool", userptr);
    vkUpdateDescriptorSetWithTemplate = (PFN_vkUpdateDescriptorSetWithTemplate) load("vkUpdateDescriptorSetWithTemplate", userptr);
}
static void glad_vk_load_VK_EXT_debug_report( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_debug_report) return;
    vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) load("vkCreateDebugReportCallbackEXT", userptr);
    vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT) load("vkDebugReportMessageEXT", userptr);
    vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) load("vkDestroyDebugReportCallbackEXT", userptr);
}
static void glad_vk_load_VK_KHR_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_surface) return;
    vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR) load("vkDestroySurfaceKHR", userptr);
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) load("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", userptr);
    vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) load("vkGetPhysicalDeviceSurfaceFormatsKHR", userptr);
    vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) load("vkGetPhysicalDeviceSurfacePresentModesKHR", userptr);
    vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR) load("vkGetPhysicalDeviceSurfaceSupportKHR", userptr);
}
static void glad_vk_load_VK_KHR_swapchain( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_swapchain) return;
    vkAcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR) load("vkAcquireNextImage2KHR", userptr);
    vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) load("vkAcquireNextImageKHR", userptr);
    vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) load("vkCreateSwapchainKHR", userptr);
    vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR) load("vkDestroySwapchainKHR", userptr);
    vkGetDeviceGroupPresentCapabilitiesKHR = (PFN_vkGetDeviceGroupPresentCapabilitiesKHR) load("vkGetDeviceGroupPresentCapabilitiesKHR", userptr);
    vkGetDeviceGroupSurfacePresentModesKHR = (PFN_vkGetDeviceGroupSurfacePresentModesKHR) load("vkGetDeviceGroupSurfacePresentModesKHR", userptr);
    vkGetPhysicalDevicePresentRectanglesKHR = (PFN_vkGetPhysicalDevicePresentRectanglesKHR) load("vkGetPhysicalDevicePresentRectanglesKHR", userptr);
    vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) load("vkGetSwapchainImagesKHR", userptr);
    vkQueuePresentKHR = (PFN_vkQueuePresentKHR) load("vkQueuePresentKHR", userptr);
}



static int glad_vk_get_extensions( VkPhysicalDevice physical_device, uint32_t *out_extension_count, char ***out_extensions) {
    uint32_t i;
    uint32_t instance_extension_count = 0;
    uint32_t device_extension_count = 0;
    uint32_t max_extension_count;
    uint32_t total_extension_count;
    char **extensions;
    VkExtensionProperties *ext_properties;
    VkResult result;

    if (vkEnumerateInstanceExtensionProperties == NULL || (physical_device != NULL && vkEnumerateDeviceExtensionProperties == NULL)) {
        return 0;
    }

    result = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
    if (result != VK_SUCCESS) {
        return 0;
    }

    if (physical_device != NULL) {
        result = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extension_count, NULL);
        if (result != VK_SUCCESS) {
            return 0;
        }
    }

    total_extension_count = instance_extension_count + device_extension_count;
    max_extension_count = instance_extension_count > device_extension_count
        ? instance_extension_count : device_extension_count;

    ext_properties = (VkExtensionProperties*) malloc(max_extension_count * sizeof(VkExtensionProperties));
    if (ext_properties == NULL) {
        return 0;
    }

    result = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, ext_properties);
    if (result != VK_SUCCESS) {
        free((void*) ext_properties);
        return 0;
    }

    extensions = (char**) calloc(total_extension_count, sizeof(char*));
    if (extensions == NULL) {
        free((void*) ext_properties);
        return 0;
    }

    for (i = 0; i < instance_extension_count; ++i) {
        VkExtensionProperties ext = ext_properties[i];

        size_t extension_name_length = strlen(ext.extensionName) + 1;
        extensions[i] = (char*) malloc(extension_name_length * sizeof(char));
        memcpy(extensions[i], ext.extensionName, extension_name_length * sizeof(char));
    }

    if (physical_device != NULL) {
        result = vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extension_count, ext_properties);
        if (result != VK_SUCCESS) {
            for (i = 0; i < instance_extension_count; ++i) {
                free((void*) extensions[i]);
            }
            free(extensions);
            return 0;
        }

        for (i = 0; i < device_extension_count; ++i) {
            VkExtensionProperties ext = ext_properties[i];

            size_t extension_name_length = strlen(ext.extensionName) + 1;
            extensions[instance_extension_count + i] = (char*) malloc(extension_name_length * sizeof(char));
            memcpy(extensions[instance_extension_count + i], ext.extensionName, extension_name_length * sizeof(char));
        }
    }

    free((void*) ext_properties);

    *out_extension_count = total_extension_count;
    *out_extensions = extensions;

    return 1;
}

static void glad_vk_free_extensions(uint32_t extension_count, char **extensions) {
    uint32_t i;

    for(i = 0; i < extension_count ; ++i) {
        free((void*) (extensions[i]));
    }

    free((void*) extensions);
}

static int glad_vk_has_extension(const char *name, uint32_t extension_count, char **extensions) {
    uint32_t i;

    for (i = 0; i < extension_count; ++i) {
        if(strcmp(name, extensions[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

static GLADapiproc glad_vk_get_proc_from_userptr(const char* name, void *userptr) {
    return (GLAD_GNUC_EXTENSION (GLADapiproc (*)(const char *name)) userptr)(name);
}

static int glad_vk_find_extensions_vulkan( VkPhysicalDevice physical_device) {
    uint32_t extension_count = 0;
    char **extensions = NULL;
    if (!glad_vk_get_extensions(physical_device, &extension_count, &extensions)) return 0;

    GLAD_VK_EXT_debug_report = glad_vk_has_extension("VK_EXT_debug_report", extension_count, extensions);
    GLAD_VK_KHR_surface = glad_vk_has_extension("VK_KHR_surface", extension_count, extensions);
    GLAD_VK_KHR_swapchain = glad_vk_has_extension("VK_KHR_swapchain", extension_count, extensions);

    glad_vk_free_extensions(extension_count, extensions);

    return 1;
}

static int glad_vk_find_core_vulkan( VkPhysicalDevice physical_device) {
    int major = 1;
    int minor = 0;

#ifdef VK_VERSION_1_1
    if (vkEnumerateInstanceVersion  != NULL) {
        uint32_t version;
        VkResult result;

        result = vkEnumerateInstanceVersion(&version);
        if (result == VK_SUCCESS) {
            major = (int) VK_VERSION_MAJOR(version);
            minor = (int) VK_VERSION_MINOR(version);
        }
    }
#endif

    if (physical_device != NULL && vkGetPhysicalDeviceProperties  != NULL) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_device, &properties);

        major = (int) VK_VERSION_MAJOR(properties.apiVersion);
        minor = (int) VK_VERSION_MINOR(properties.apiVersion);
    }

    GLAD_VK_VERSION_1_0 = (major == 1 && minor >= 0) || major > 1;
    GLAD_VK_VERSION_1_1 = (major == 1 && minor >= 1) || major > 1;

    return GLAD_MAKE_VERSION(major, minor);
}

int gladLoadVulkanUserPtr( VkPhysicalDevice physical_device, GLADuserptrloadfunc load, void *userptr) {
    int version;

#ifdef VK_VERSION_1_1
    vkEnumerateInstanceVersion  = (PFN_vkEnumerateInstanceVersion) load("vkEnumerateInstanceVersion", userptr);
#endif
    version = glad_vk_find_core_vulkan( physical_device);
    if (!version) {
        return 0;
    }

    glad_vk_load_VK_VERSION_1_0(load, userptr);
    glad_vk_load_VK_VERSION_1_1(load, userptr);

    if (!glad_vk_find_extensions_vulkan( physical_device)) return 0;
    glad_vk_load_VK_EXT_debug_report(load, userptr);
    glad_vk_load_VK_KHR_surface(load, userptr);
    glad_vk_load_VK_KHR_swapchain(load, userptr);


    return version;
}


int gladLoadVulkan( VkPhysicalDevice physical_device, GLADloadfunc load) {
    return gladLoadVulkanUserPtr( physical_device, glad_vk_get_proc_from_userptr, GLAD_GNUC_EXTENSION (void*) load);
}




