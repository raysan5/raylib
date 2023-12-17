/*
 * Copyright (c) 2015-2016 The Khronos Group Inc.
 * Copyright (c) 2015-2016 Valve Corporation
 * Copyright (c) 2015-2016 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Cody Northrop <cody@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Ian Elliott <ian@LunarG.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Piers Daniell <pdaniell@nvidia.com>
 * Author: Gwan-gyeong Mun <elongbug@gmail.com>
 * Porter: Camilla Löwy <elmindreda@glfw.org>
 */
/*
 * Draw a textured triangle with depth testing.  This is written against Intel
 * ICD.  It does not do state transition nor object memory binding like it
 * should.  It also does no error checking.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <signal.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <glad/vulkan.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define DEMO_TEXTURE_COUNT 1
#define VERTEX_BUFFER_BIND_ID 0
#define APP_SHORT_NAME "tri"
#define APP_LONG_NAME "The Vulkan Triangle Demo Program"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#if defined(NDEBUG) && defined(__GNUC__)
#define U_ASSERT_ONLY __attribute__((unused))
#else
#define U_ASSERT_ONLY
#endif

#define ERR_EXIT(err_msg, err_class)                                           \
    do {                                                                       \
        printf(err_msg);                                                       \
        fflush(stdout);                                                        \
        exit(1);                                                               \
    } while (0)

static const uint32_t fragShaderCode[] = {
    0x07230203,0x00010000,0x00080007,0x00000014,0x00000000,0x00020011,0x00000001,0x0006000b,
    0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
    0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x00000011,0x00030010,
    0x00000004,0x00000007,0x00030003,0x00000002,0x00000190,0x00090004,0x415f4c47,0x735f4252,
    0x72617065,0x5f657461,0x64616873,0x6f5f7265,0x63656a62,0x00007374,0x00090004,0x415f4c47,
    0x735f4252,0x69646168,0x6c5f676e,0x75676e61,0x5f656761,0x70303234,0x006b6361,0x00040005,
    0x00000004,0x6e69616d,0x00000000,0x00050005,0x00000009,0x61724675,0x6c6f4367,0x0000726f,
    0x00030005,0x0000000d,0x00786574,0x00050005,0x00000011,0x63786574,0x64726f6f,0x00000000,
    0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x00000022,0x00000000,
    0x00040047,0x0000000d,0x00000021,0x00000000,0x00040047,0x00000011,0x0000001e,0x00000000,
    0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,0x00000020,
    0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,0x00000007,
    0x0004003b,0x00000008,0x00000009,0x00000003,0x00090019,0x0000000a,0x00000006,0x00000001,
    0x00000000,0x00000000,0x00000000,0x00000001,0x00000000,0x0003001b,0x0000000b,0x0000000a,
    0x00040020,0x0000000c,0x00000000,0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000000,
    0x00040017,0x0000000f,0x00000006,0x00000002,0x00040020,0x00000010,0x00000001,0x0000000f,
    0x0004003b,0x00000010,0x00000011,0x00000001,0x00050036,0x00000002,0x00000004,0x00000000,
    0x00000003,0x000200f8,0x00000005,0x0004003d,0x0000000b,0x0000000e,0x0000000d,0x0004003d,
    0x0000000f,0x00000012,0x00000011,0x00050057,0x00000007,0x00000013,0x0000000e,0x00000012,
    0x0003003e,0x00000009,0x00000013,0x000100fd,0x00010038
};

static const uint32_t vertShaderCode[] = {
    0x07230203,0x00010000,0x00080007,0x00000018,0x00000000,0x00020011,0x00000001,0x0006000b,
    0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
    0x0009000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000b,0x00000010,
    0x00000014,0x00030003,0x00000002,0x00000190,0x00090004,0x415f4c47,0x735f4252,0x72617065,
    0x5f657461,0x64616873,0x6f5f7265,0x63656a62,0x00007374,0x00090004,0x415f4c47,0x735f4252,
    0x69646168,0x6c5f676e,0x75676e61,0x5f656761,0x70303234,0x006b6361,0x00040005,0x00000004,
    0x6e69616d,0x00000000,0x00050005,0x00000009,0x63786574,0x64726f6f,0x00000000,0x00040005,
    0x0000000b,0x72747461,0x00000000,0x00060005,0x0000000e,0x505f6c67,0x65567265,0x78657472,
    0x00000000,0x00060006,0x0000000e,0x00000000,0x505f6c67,0x7469736f,0x006e6f69,0x00030005,
    0x00000010,0x00000000,0x00030005,0x00000014,0x00736f70,0x00040047,0x00000009,0x0000001e,
    0x00000000,0x00040047,0x0000000b,0x0000001e,0x00000001,0x00050048,0x0000000e,0x00000000,
    0x0000000b,0x00000000,0x00030047,0x0000000e,0x00000002,0x00040047,0x00000014,0x0000001e,
    0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
    0x00000020,0x00040017,0x00000007,0x00000006,0x00000002,0x00040020,0x00000008,0x00000003,
    0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040020,0x0000000a,0x00000001,
    0x00000007,0x0004003b,0x0000000a,0x0000000b,0x00000001,0x00040017,0x0000000d,0x00000006,
    0x00000004,0x0003001e,0x0000000e,0x0000000d,0x00040020,0x0000000f,0x00000003,0x0000000e,
    0x0004003b,0x0000000f,0x00000010,0x00000003,0x00040015,0x00000011,0x00000020,0x00000001,
    0x0004002b,0x00000011,0x00000012,0x00000000,0x00040020,0x00000013,0x00000001,0x0000000d,
    0x0004003b,0x00000013,0x00000014,0x00000001,0x00040020,0x00000016,0x00000003,0x0000000d,
    0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,0x00000005,0x0004003d,
    0x00000007,0x0000000c,0x0000000b,0x0003003e,0x00000009,0x0000000c,0x0004003d,0x0000000d,
    0x00000015,0x00000014,0x00050041,0x00000016,0x00000017,0x00000010,0x00000012,0x0003003e,
    0x00000017,0x00000015,0x000100fd,0x00010038
};

struct texture_object {
    VkSampler sampler;

    VkImage image;
    VkImageLayout imageLayout;

    VkDeviceMemory mem;
    VkImageView view;
    int32_t tex_width, tex_height;
};

static int validation_error = 0;

VKAPI_ATTR VkBool32 VKAPI_CALL
BreakCallback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
              uint64_t srcObject, size_t location, int32_t msgCode,
              const char *pLayerPrefix, const char *pMsg,
              void *pUserData) {
#ifdef _WIN32
    DebugBreak();
#else
    raise(SIGTRAP);
#endif

    return false;
}

typedef struct {
    VkImage image;
    VkCommandBuffer cmd;
    VkImageView view;
} SwapchainBuffers;

struct demo {
    GLFWwindow* window;
    VkSurfaceKHR surface;
    bool use_staging_buffer;

    VkInstance inst;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkQueue queue;
    VkPhysicalDeviceProperties gpu_props;
    VkPhysicalDeviceFeatures gpu_features;
    VkQueueFamilyProperties *queue_props;
    uint32_t graphics_queue_node_index;

    uint32_t enabled_extension_count;
    uint32_t enabled_layer_count;
    const char *extension_names[64];
    const char *enabled_layers[64];

    int width, height;
    VkFormat format;
    VkColorSpaceKHR color_space;

    uint32_t swapchainImageCount;
    VkSwapchainKHR swapchain;
    SwapchainBuffers *buffers;

    VkCommandPool cmd_pool;

    struct {
        VkFormat format;

        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } depth;

    struct texture_object textures[DEMO_TEXTURE_COUNT];

    struct {
        VkBuffer buf;
        VkDeviceMemory mem;

        VkPipelineVertexInputStateCreateInfo vi;
        VkVertexInputBindingDescription vi_bindings[1];
        VkVertexInputAttributeDescription vi_attrs[2];
    } vertices;

    VkCommandBuffer setup_cmd; // Command Buffer for initialization commands
    VkCommandBuffer draw_cmd;  // Command Buffer for drawing commands
    VkPipelineLayout pipeline_layout;
    VkDescriptorSetLayout desc_layout;
    VkPipelineCache pipelineCache;
    VkRenderPass render_pass;
    VkPipeline pipeline;

    VkShaderModule vert_shader_module;
    VkShaderModule frag_shader_module;

    VkDescriptorPool desc_pool;
    VkDescriptorSet desc_set;

    VkFramebuffer *framebuffers;

    VkPhysicalDeviceMemoryProperties memory_properties;

    int32_t curFrame;
    int32_t frameCount;
    bool validate;
    bool use_break;
    VkDebugReportCallbackEXT msg_callback;

    float depthStencil;
    float depthIncrement;

    uint32_t current_buffer;
    uint32_t queue_count;
};

VKAPI_ATTR VkBool32 VKAPI_CALL
dbgFunc(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
    uint64_t srcObject, size_t location, int32_t msgCode,
    const char *pLayerPrefix, const char *pMsg, void *pUserData) {
    char *message = (char *)malloc(strlen(pMsg) + 100);

    assert(message);

    validation_error = 1;

    if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        sprintf(message, "ERROR: [%s] Code %d : %s", pLayerPrefix, msgCode,
            pMsg);
    } else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        sprintf(message, "WARNING: [%s] Code %d : %s", pLayerPrefix, msgCode,
            pMsg);
    } else {
        return false;
    }

    printf("%s\n", message);
    fflush(stdout);
    free(message);

    /*
    * false indicates that layer should not bail-out of an
    * API call that had validation failures. This may mean that the
    * app dies inside the driver due to invalid parameter(s).
    * That's what would happen without validation layers, so we'll
    * keep that behavior here.
    */
    return false;
}

// Forward declaration:
static void demo_resize(struct demo *demo);

static bool memory_type_from_properties(struct demo *demo, uint32_t typeBits,
                                        VkFlags requirements_mask,
                                        uint32_t *typeIndex) {
    uint32_t i;
    // Search memtypes to find first index with those properties
    for (i = 0; i < VK_MAX_MEMORY_TYPES; i++) {
        if ((typeBits & 1) == 1) {
            // Type is available, does it match user properties?
            if ((demo->memory_properties.memoryTypes[i].propertyFlags &
                 requirements_mask) == requirements_mask) {
                *typeIndex = i;
                return true;
            }
        }
        typeBits >>= 1;
    }
    // No memory types matched, return failure
    return false;
}

static void demo_flush_init_cmd(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;

    if (demo->setup_cmd == VK_NULL_HANDLE)
        return;

    err = vkEndCommandBuffer(demo->setup_cmd);
    assert(!err);

    const VkCommandBuffer cmd_bufs[] = {demo->setup_cmd};
    VkFence nullFence = {VK_NULL_HANDLE};
    VkSubmitInfo submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                .pNext = NULL,
                                .waitSemaphoreCount = 0,
                                .pWaitSemaphores = NULL,
                                .pWaitDstStageMask = NULL,
                                .commandBufferCount = 1,
                                .pCommandBuffers = cmd_bufs,
                                .signalSemaphoreCount = 0,
                                .pSignalSemaphores = NULL};

    err = vkQueueSubmit(demo->queue, 1, &submit_info, nullFence);
    assert(!err);

    err = vkQueueWaitIdle(demo->queue);
    assert(!err);

    vkFreeCommandBuffers(demo->device, demo->cmd_pool, 1, cmd_bufs);
    demo->setup_cmd = VK_NULL_HANDLE;
}

static void demo_set_image_layout(struct demo *demo, VkImage image,
                                  VkImageAspectFlags aspectMask,
                                  VkImageLayout old_image_layout,
                                  VkImageLayout new_image_layout,
                                  VkAccessFlagBits srcAccessMask) {

    VkResult U_ASSERT_ONLY err;

    if (demo->setup_cmd == VK_NULL_HANDLE) {
        const VkCommandBufferAllocateInfo cmd = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = NULL,
            .commandPool = demo->cmd_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        err = vkAllocateCommandBuffers(demo->device, &cmd, &demo->setup_cmd);
        assert(!err);

        VkCommandBufferBeginInfo cmd_buf_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = NULL,
            .flags = 0,
            .pInheritanceInfo = NULL,
        };
        err = vkBeginCommandBuffer(demo->setup_cmd, &cmd_buf_info);
        assert(!err);
    }

    VkImageMemoryBarrier image_memory_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = srcAccessMask,
        .dstAccessMask = 0,
        .oldLayout = old_image_layout,
        .newLayout = new_image_layout,
        .image = image,
        .subresourceRange = {aspectMask, 0, 1, 0, 1}};

    if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        /* Make sure anything that was copying from this image has completed */
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    }

    if (new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        image_memory_barrier.dstAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    if (new_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        image_memory_barrier.dstAccessMask =
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    if (new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        /* Make sure any Copy or CPU writes to image are flushed */
        image_memory_barrier.dstAccessMask =
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    }

    VkImageMemoryBarrier *pmemory_barrier = &image_memory_barrier;

    VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    vkCmdPipelineBarrier(demo->setup_cmd, src_stages, dest_stages, 0, 0, NULL,
                         0, NULL, 1, pmemory_barrier);
}

static void demo_draw_build_cmd(struct demo *demo) {
    const VkCommandBufferBeginInfo cmd_buf_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = 0,
        .pInheritanceInfo = NULL,
    };
    const VkClearValue clear_values[2] = {
            [0] = {.color.float32 = {0.2f, 0.2f, 0.2f, 0.2f}},
            [1] = {.depthStencil = {demo->depthStencil, 0}},
    };
    const VkRenderPassBeginInfo rp_begin = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = NULL,
        .renderPass = demo->render_pass,
        .framebuffer = demo->framebuffers[demo->current_buffer],
        .renderArea.offset.x = 0,
        .renderArea.offset.y = 0,
        .renderArea.extent.width = demo->width,
        .renderArea.extent.height = demo->height,
        .clearValueCount = 2,
        .pClearValues = clear_values,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkBeginCommandBuffer(demo->draw_cmd, &cmd_buf_info);
    assert(!err);

    // We can use LAYOUT_UNDEFINED as a wildcard here because we don't care what
    // happens to the previous contents of the image
    VkImageMemoryBarrier image_memory_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = demo->buffers[demo->current_buffer].image,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

    vkCmdPipelineBarrier(demo->draw_cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0,
                         NULL, 1, &image_memory_barrier);
    vkCmdBeginRenderPass(demo->draw_cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(demo->draw_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      demo->pipeline);
    vkCmdBindDescriptorSets(demo->draw_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            demo->pipeline_layout, 0, 1, &demo->desc_set, 0,
                            NULL);

    VkViewport viewport;
    memset(&viewport, 0, sizeof(viewport));
    viewport.height = (float)demo->height;
    viewport.width = (float)demo->width;
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;
    vkCmdSetViewport(demo->draw_cmd, 0, 1, &viewport);

    VkRect2D scissor;
    memset(&scissor, 0, sizeof(scissor));
    scissor.extent.width = demo->width;
    scissor.extent.height = demo->height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(demo->draw_cmd, 0, 1, &scissor);

    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(demo->draw_cmd, VERTEX_BUFFER_BIND_ID, 1,
                           &demo->vertices.buf, offsets);

    vkCmdDraw(demo->draw_cmd, 3, 1, 0, 0);
    vkCmdEndRenderPass(demo->draw_cmd);

    VkImageMemoryBarrier prePresentBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = NULL,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

    prePresentBarrier.image = demo->buffers[demo->current_buffer].image;
    VkImageMemoryBarrier *pmemory_barrier = &prePresentBarrier;
    vkCmdPipelineBarrier(demo->draw_cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0,
                         NULL, 1, pmemory_barrier);

    err = vkEndCommandBuffer(demo->draw_cmd);
    assert(!err);
}

static void demo_draw(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;
    VkSemaphore imageAcquiredSemaphore, drawCompleteSemaphore;
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
    };

    err = vkCreateSemaphore(demo->device, &semaphoreCreateInfo,
                            NULL, &imageAcquiredSemaphore);
    assert(!err);

    err = vkCreateSemaphore(demo->device, &semaphoreCreateInfo,
                            NULL, &drawCompleteSemaphore);
    assert(!err);

    // Get the index of the next available swapchain image:
    err = vkAcquireNextImageKHR(demo->device, demo->swapchain, UINT64_MAX,
                                imageAcquiredSemaphore,
                                (VkFence)0, // TODO: Show use of fence
                                &demo->current_buffer);
    if (err == VK_ERROR_OUT_OF_DATE_KHR) {
        // demo->swapchain is out of date (e.g. the window was resized) and
        // must be recreated:
        demo_resize(demo);
        demo_draw(demo);
        vkDestroySemaphore(demo->device, imageAcquiredSemaphore, NULL);
        vkDestroySemaphore(demo->device, drawCompleteSemaphore, NULL);
        return;
    } else if (err == VK_SUBOPTIMAL_KHR) {
        // demo->swapchain is not as optimal as it could be, but the platform's
        // presentation engine will still present the image correctly.
    } else {
        assert(!err);
    }

    demo_flush_init_cmd(demo);

    // Wait for the present complete semaphore to be signaled to ensure
    // that the image won't be rendered to until the presentation
    // engine has fully released ownership to the application, and it is
    // okay to render to the image.

    demo_draw_build_cmd(demo);
    VkFence nullFence = VK_NULL_HANDLE;
    VkPipelineStageFlags pipe_stage_flags =
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                .pNext = NULL,
                                .waitSemaphoreCount = 1,
                                .pWaitSemaphores = &imageAcquiredSemaphore,
                                .pWaitDstStageMask = &pipe_stage_flags,
                                .commandBufferCount = 1,
                                .pCommandBuffers = &demo->draw_cmd,
                                .signalSemaphoreCount = 1,
                                .pSignalSemaphores = &drawCompleteSemaphore};

    err = vkQueueSubmit(demo->queue, 1, &submit_info, nullFence);
    assert(!err);

    VkPresentInfoKHR present = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &drawCompleteSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &demo->swapchain,
        .pImageIndices = &demo->current_buffer,
    };

    err = vkQueuePresentKHR(demo->queue, &present);
    if (err == VK_ERROR_OUT_OF_DATE_KHR) {
        // demo->swapchain is out of date (e.g. the window was resized) and
        // must be recreated:
        demo_resize(demo);
    } else if (err == VK_SUBOPTIMAL_KHR) {
        // demo->swapchain is not as optimal as it could be, but the platform's
        // presentation engine will still present the image correctly.
    } else {
        assert(!err);
    }

    err = vkQueueWaitIdle(demo->queue);
    assert(err == VK_SUCCESS);

    vkDestroySemaphore(demo->device, imageAcquiredSemaphore, NULL);
    vkDestroySemaphore(demo->device, drawCompleteSemaphore, NULL);
}

static void demo_prepare_buffers(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;
    VkSwapchainKHR oldSwapchain = demo->swapchain;

    // Check the surface capabilities and formats
    VkSurfaceCapabilitiesKHR surfCapabilities;
    err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        demo->gpu, demo->surface, &surfCapabilities);
    assert(!err);

    uint32_t presentModeCount;
    err = vkGetPhysicalDeviceSurfacePresentModesKHR(
        demo->gpu, demo->surface, &presentModeCount, NULL);
    assert(!err);
    VkPresentModeKHR *presentModes =
        (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
    assert(presentModes);
    err = vkGetPhysicalDeviceSurfacePresentModesKHR(
        demo->gpu, demo->surface, &presentModeCount, presentModes);
    assert(!err);

    VkExtent2D swapchainExtent;
    // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
    if (surfCapabilities.currentExtent.width == 0xFFFFFFFF) {
        // If the surface size is undefined, the size is set to the size
        // of the images requested, which must fit within the minimum and
        // maximum values.
        swapchainExtent.width = demo->width;
        swapchainExtent.height = demo->height;

        if (swapchainExtent.width < surfCapabilities.minImageExtent.width) {
            swapchainExtent.width = surfCapabilities.minImageExtent.width;
        } else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width) {
            swapchainExtent.width = surfCapabilities.maxImageExtent.width;
        }

        if (swapchainExtent.height < surfCapabilities.minImageExtent.height) {
            swapchainExtent.height = surfCapabilities.minImageExtent.height;
        } else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height) {
            swapchainExtent.height = surfCapabilities.maxImageExtent.height;
        }
    } else {
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfCapabilities.currentExtent;
        demo->width = surfCapabilities.currentExtent.width;
        demo->height = surfCapabilities.currentExtent.height;
    }

    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    // Determine the number of VkImage's to use in the swap chain.
    // Application desires to only acquire 1 image at a time (which is
    // "surfCapabilities.minImageCount").
    uint32_t desiredNumOfSwapchainImages = surfCapabilities.minImageCount;
    // If maxImageCount is 0, we can ask for as many images as we want;
    // otherwise we're limited to maxImageCount
    if ((surfCapabilities.maxImageCount > 0) &&
        (desiredNumOfSwapchainImages > surfCapabilities.maxImageCount)) {
        // Application must settle for fewer images than desired:
        desiredNumOfSwapchainImages = surfCapabilities.maxImageCount;
    }

    VkSurfaceTransformFlagsKHR preTransform;
    if (surfCapabilities.supportedTransforms &
        VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        preTransform = surfCapabilities.currentTransform;
    }

    const VkSwapchainCreateInfoKHR swapchain = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = NULL,
        .surface = demo->surface,
        .minImageCount = desiredNumOfSwapchainImages,
        .imageFormat = demo->format,
        .imageColorSpace = demo->color_space,
        .imageExtent =
            {
             .width = swapchainExtent.width, .height = swapchainExtent.height,
            },
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = preTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .imageArrayLayers = 1,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = NULL,
        .presentMode = swapchainPresentMode,
        .oldSwapchain = oldSwapchain,
        .clipped = true,
    };
    uint32_t i;

    err = vkCreateSwapchainKHR(demo->device, &swapchain, NULL, &demo->swapchain);
    assert(!err);

    // If we just re-created an existing swapchain, we should destroy the old
    // swapchain at this point.
    // Note: destroying the swapchain also cleans up all its associated
    // presentable images once the platform is done with them.
    if (oldSwapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(demo->device, oldSwapchain, NULL);
    }

    err = vkGetSwapchainImagesKHR(demo->device, demo->swapchain,
                                        &demo->swapchainImageCount, NULL);
    assert(!err);

    VkImage *swapchainImages =
        (VkImage *)malloc(demo->swapchainImageCount * sizeof(VkImage));
    assert(swapchainImages);
    err = vkGetSwapchainImagesKHR(demo->device, demo->swapchain,
                                  &demo->swapchainImageCount,
                                  swapchainImages);
    assert(!err);

    demo->buffers = (SwapchainBuffers *)malloc(sizeof(SwapchainBuffers) *
                                               demo->swapchainImageCount);
    assert(demo->buffers);

    for (i = 0; i < demo->swapchainImageCount; i++) {
        VkImageViewCreateInfo color_attachment_view = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .format = demo->format,
            .components =
                {
                 .r = VK_COMPONENT_SWIZZLE_R,
                 .g = VK_COMPONENT_SWIZZLE_G,
                 .b = VK_COMPONENT_SWIZZLE_B,
                 .a = VK_COMPONENT_SWIZZLE_A,
                },
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                 .baseMipLevel = 0,
                                 .levelCount = 1,
                                 .baseArrayLayer = 0,
                                 .layerCount = 1},
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .flags = 0,
        };

        demo->buffers[i].image = swapchainImages[i];

        color_attachment_view.image = demo->buffers[i].image;

        err = vkCreateImageView(demo->device, &color_attachment_view, NULL,
                                &demo->buffers[i].view);
        assert(!err);
    }

    demo->current_buffer = 0;

    if (NULL != presentModes) {
        free(presentModes);
    }
}

static void demo_prepare_depth(struct demo *demo) {
    const VkFormat depth_format = VK_FORMAT_D16_UNORM;
    const VkImageCreateInfo image = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = NULL,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depth_format,
        .extent = {demo->width, demo->height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .flags = 0,
    };
    VkMemoryAllocateInfo mem_alloc = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = 0,
        .memoryTypeIndex = 0,
    };
    VkImageViewCreateInfo view = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = NULL,
        .image = VK_NULL_HANDLE,
        .format = depth_format,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1},
        .flags = 0,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
    };

    VkMemoryRequirements mem_reqs;
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;

    demo->depth.format = depth_format;

    /* create image */
    err = vkCreateImage(demo->device, &image, NULL, &demo->depth.image);
    assert(!err);

    /* get memory requirements for this object */
    vkGetImageMemoryRequirements(demo->device, demo->depth.image, &mem_reqs);

    /* select memory size and type */
    mem_alloc.allocationSize = mem_reqs.size;
    pass = memory_type_from_properties(demo, mem_reqs.memoryTypeBits,
                                       0, /* No requirements */
                                       &mem_alloc.memoryTypeIndex);
    assert(pass);

    /* allocate memory */
    err = vkAllocateMemory(demo->device, &mem_alloc, NULL, &demo->depth.mem);
    assert(!err);

    /* bind memory */
    err =
        vkBindImageMemory(demo->device, demo->depth.image, demo->depth.mem, 0);
    assert(!err);

    demo_set_image_layout(demo, demo->depth.image, VK_IMAGE_ASPECT_DEPTH_BIT,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                          0);

    /* create image view */
    view.image = demo->depth.image;
    err = vkCreateImageView(demo->device, &view, NULL, &demo->depth.view);
    assert(!err);
}

static void
demo_prepare_texture_image(struct demo *demo, const uint32_t *tex_colors,
                           struct texture_object *tex_obj, VkImageTiling tiling,
                           VkImageUsageFlags usage, VkFlags required_props) {
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 2;
    const int32_t tex_height = 2;
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;

    tex_obj->tex_width = tex_width;
    tex_obj->tex_height = tex_height;

    const VkImageCreateInfo image_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = NULL,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = tex_format,
        .extent = {tex_width, tex_height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling,
        .usage = usage,
        .flags = 0,
        .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED
    };
    VkMemoryAllocateInfo mem_alloc = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = 0,
        .memoryTypeIndex = 0,
    };

    VkMemoryRequirements mem_reqs;

    err =
        vkCreateImage(demo->device, &image_create_info, NULL, &tex_obj->image);
    assert(!err);

    vkGetImageMemoryRequirements(demo->device, tex_obj->image, &mem_reqs);

    mem_alloc.allocationSize = mem_reqs.size;
    pass =
        memory_type_from_properties(demo, mem_reqs.memoryTypeBits,
                                    required_props, &mem_alloc.memoryTypeIndex);
    assert(pass);

    /* allocate memory */
    err = vkAllocateMemory(demo->device, &mem_alloc, NULL, &tex_obj->mem);
    assert(!err);

    /* bind memory */
    err = vkBindImageMemory(demo->device, tex_obj->image, tex_obj->mem, 0);
    assert(!err);

    if (required_props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        const VkImageSubresource subres = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .arrayLayer = 0,
        };
        VkSubresourceLayout layout;
        void *data;
        int32_t x, y;

        vkGetImageSubresourceLayout(demo->device, tex_obj->image, &subres,
                                    &layout);

        err = vkMapMemory(demo->device, tex_obj->mem, 0,
                          mem_alloc.allocationSize, 0, &data);
        assert(!err);

        for (y = 0; y < tex_height; y++) {
            uint32_t *row = (uint32_t *)((char *)data + layout.rowPitch * y);
            for (x = 0; x < tex_width; x++)
                row[x] = tex_colors[(x & 1) ^ (y & 1)];
        }

        vkUnmapMemory(demo->device, tex_obj->mem);
    }

    tex_obj->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    demo_set_image_layout(demo, tex_obj->image, VK_IMAGE_ASPECT_COLOR_BIT,
                          VK_IMAGE_LAYOUT_PREINITIALIZED, tex_obj->imageLayout,
                          VK_ACCESS_HOST_WRITE_BIT);
    /* setting the image layout does not reference the actual memory so no need
     * to add a mem ref */
}

static void demo_destroy_texture_image(struct demo *demo,
                                       struct texture_object *tex_obj) {
    /* clean up staging resources */
    vkDestroyImage(demo->device, tex_obj->image, NULL);
    vkFreeMemory(demo->device, tex_obj->mem, NULL);
}

static void demo_prepare_textures(struct demo *demo) {
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkFormatProperties props;
    const uint32_t tex_colors[DEMO_TEXTURE_COUNT][2] = {
        {0xffff0000, 0xff00ff00},
    };
    uint32_t i;
    VkResult U_ASSERT_ONLY err;

    vkGetPhysicalDeviceFormatProperties(demo->gpu, tex_format, &props);

    for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        if ((props.linearTilingFeatures &
             VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) &&
            !demo->use_staging_buffer) {
            /* Device can texture using linear textures */
            demo_prepare_texture_image(
                demo, tex_colors[i], &demo->textures[i], VK_IMAGE_TILING_LINEAR,
                VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        } else if (props.optimalTilingFeatures &
                   VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
            /* Must use staging buffer to copy linear texture to optimized */
            struct texture_object staging_texture;

            memset(&staging_texture, 0, sizeof(staging_texture));
            demo_prepare_texture_image(
                demo, tex_colors[i], &staging_texture, VK_IMAGE_TILING_LINEAR,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            demo_prepare_texture_image(
                demo, tex_colors[i], &demo->textures[i],
                VK_IMAGE_TILING_OPTIMAL,
                (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            demo_set_image_layout(demo, staging_texture.image,
                                  VK_IMAGE_ASPECT_COLOR_BIT,
                                  staging_texture.imageLayout,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                  0);

            demo_set_image_layout(demo, demo->textures[i].image,
                                  VK_IMAGE_ASPECT_COLOR_BIT,
                                  demo->textures[i].imageLayout,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  0);

            VkImageCopy copy_region = {
                .srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                .srcOffset = {0, 0, 0},
                .dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
                .dstOffset = {0, 0, 0},
                .extent = {staging_texture.tex_width,
                           staging_texture.tex_height, 1},
            };
            vkCmdCopyImage(
                demo->setup_cmd, staging_texture.image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, demo->textures[i].image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

            demo_set_image_layout(demo, demo->textures[i].image,
                                  VK_IMAGE_ASPECT_COLOR_BIT,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  demo->textures[i].imageLayout,
                                  0);

            demo_flush_init_cmd(demo);

            demo_destroy_texture_image(demo, &staging_texture);
        } else {
            /* Can't support VK_FORMAT_B8G8R8A8_UNORM !? */
            assert(!"No support for B8G8R8A8_UNORM as texture image format");
        }

        const VkSamplerCreateInfo sampler = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = NULL,
            .magFilter = VK_FILTER_NEAREST,
            .minFilter = VK_FILTER_NEAREST,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 1,
            .compareOp = VK_COMPARE_OP_NEVER,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
            .unnormalizedCoordinates = VK_FALSE,
        };
        VkImageViewCreateInfo view = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = NULL,
            .image = VK_NULL_HANDLE,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = tex_format,
            .components =
                {
                 VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                 VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A,
                },
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
            .flags = 0,
        };

        /* create sampler */
        err = vkCreateSampler(demo->device, &sampler, NULL,
                              &demo->textures[i].sampler);
        assert(!err);

        /* create image view */
        view.image = demo->textures[i].image;
        err = vkCreateImageView(demo->device, &view, NULL,
                                &demo->textures[i].view);
        assert(!err);
    }
}

static void demo_prepare_vertices(struct demo *demo) {
    // clang-format off
    const float vb[3][5] = {
        /*      position             texcoord */
        { -1.0f, -1.0f,  0.25f,     0.0f, 0.0f },
        {  1.0f, -1.0f,  0.25f,     1.0f, 0.0f },
        {  0.0f,  1.0f,  1.0f,      0.5f, 1.0f },
    };
    // clang-format on
    const VkBufferCreateInfo buf_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = NULL,
        .size = sizeof(vb),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .flags = 0,
    };
    VkMemoryAllocateInfo mem_alloc = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .allocationSize = 0,
        .memoryTypeIndex = 0,
    };
    VkMemoryRequirements mem_reqs;
    VkResult U_ASSERT_ONLY err;
    bool U_ASSERT_ONLY pass;
    void *data;

    memset(&demo->vertices, 0, sizeof(demo->vertices));

    err = vkCreateBuffer(demo->device, &buf_info, NULL, &demo->vertices.buf);
    assert(!err);

    vkGetBufferMemoryRequirements(demo->device, demo->vertices.buf, &mem_reqs);
    assert(!err);

    mem_alloc.allocationSize = mem_reqs.size;
    pass = memory_type_from_properties(demo, mem_reqs.memoryTypeBits,
                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                       &mem_alloc.memoryTypeIndex);
    assert(pass);

    err = vkAllocateMemory(demo->device, &mem_alloc, NULL, &demo->vertices.mem);
    assert(!err);

    err = vkMapMemory(demo->device, demo->vertices.mem, 0,
                      mem_alloc.allocationSize, 0, &data);
    assert(!err);

    memcpy(data, vb, sizeof(vb));

    vkUnmapMemory(demo->device, demo->vertices.mem);

    err = vkBindBufferMemory(demo->device, demo->vertices.buf,
                             demo->vertices.mem, 0);
    assert(!err);

    demo->vertices.vi.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    demo->vertices.vi.pNext = NULL;
    demo->vertices.vi.vertexBindingDescriptionCount = 1;
    demo->vertices.vi.pVertexBindingDescriptions = demo->vertices.vi_bindings;
    demo->vertices.vi.vertexAttributeDescriptionCount = 2;
    demo->vertices.vi.pVertexAttributeDescriptions = demo->vertices.vi_attrs;

    demo->vertices.vi_bindings[0].binding = VERTEX_BUFFER_BIND_ID;
    demo->vertices.vi_bindings[0].stride = sizeof(vb[0]);
    demo->vertices.vi_bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    demo->vertices.vi_attrs[0].binding = VERTEX_BUFFER_BIND_ID;
    demo->vertices.vi_attrs[0].location = 0;
    demo->vertices.vi_attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    demo->vertices.vi_attrs[0].offset = 0;

    demo->vertices.vi_attrs[1].binding = VERTEX_BUFFER_BIND_ID;
    demo->vertices.vi_attrs[1].location = 1;
    demo->vertices.vi_attrs[1].format = VK_FORMAT_R32G32_SFLOAT;
    demo->vertices.vi_attrs[1].offset = sizeof(float) * 3;
}

static void demo_prepare_descriptor_layout(struct demo *demo) {
    const VkDescriptorSetLayoutBinding layout_binding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = DEMO_TEXTURE_COUNT,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = NULL,
    };
    const VkDescriptorSetLayoutCreateInfo descriptor_layout = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .bindingCount = 1,
        .pBindings = &layout_binding,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkCreateDescriptorSetLayout(demo->device, &descriptor_layout, NULL,
                                      &demo->desc_layout);
    assert(!err);

    const VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .setLayoutCount = 1,
        .pSetLayouts = &demo->desc_layout,
    };

    err = vkCreatePipelineLayout(demo->device, &pPipelineLayoutCreateInfo, NULL,
                                 &demo->pipeline_layout);
    assert(!err);
}

static void demo_prepare_render_pass(struct demo *demo) {
    const VkAttachmentDescription attachments[2] = {
            [0] =
                {
                 .format = demo->format,
                 .samples = VK_SAMPLE_COUNT_1_BIT,
                 .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                 .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                 .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                 .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                 .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                 .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                },
            [1] =
                {
                 .format = demo->depth.format,
                 .samples = VK_SAMPLE_COUNT_1_BIT,
                 .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                 .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                 .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                 .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                 .initialLayout =
                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                 .finalLayout =
                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                },
    };
    const VkAttachmentReference color_reference = {
        .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    const VkAttachmentReference depth_reference = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };
    const VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .flags = 0,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_reference,
        .pResolveAttachments = NULL,
        .pDepthStencilAttachment = &depth_reference,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = NULL,
    };
    const VkRenderPassCreateInfo rp_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = NULL,
        .attachmentCount = 2,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 0,
        .pDependencies = NULL,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkCreateRenderPass(demo->device, &rp_info, NULL, &demo->render_pass);
    assert(!err);
}

static VkShaderModule
demo_prepare_shader_module(struct demo *demo, const void *code, size_t size) {
    VkShaderModuleCreateInfo moduleCreateInfo;
    VkShaderModule module;
    VkResult U_ASSERT_ONLY err;

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;

    moduleCreateInfo.codeSize = size;
    moduleCreateInfo.pCode = code;
    moduleCreateInfo.flags = 0;
    err = vkCreateShaderModule(demo->device, &moduleCreateInfo, NULL, &module);
    assert(!err);

    return module;
}

static VkShaderModule demo_prepare_vs(struct demo *demo) {
    size_t size = sizeof(vertShaderCode);

    demo->vert_shader_module =
        demo_prepare_shader_module(demo, vertShaderCode, size);

    return demo->vert_shader_module;
}

static VkShaderModule demo_prepare_fs(struct demo *demo) {
    size_t size = sizeof(fragShaderCode);

    demo->frag_shader_module =
        demo_prepare_shader_module(demo, fragShaderCode, size);

    return demo->frag_shader_module;
}

static void demo_prepare_pipeline(struct demo *demo) {
    VkGraphicsPipelineCreateInfo pipeline;
    VkPipelineCacheCreateInfo pipelineCache;

    VkPipelineVertexInputStateCreateInfo vi;
    VkPipelineInputAssemblyStateCreateInfo ia;
    VkPipelineRasterizationStateCreateInfo rs;
    VkPipelineColorBlendStateCreateInfo cb;
    VkPipelineDepthStencilStateCreateInfo ds;
    VkPipelineViewportStateCreateInfo vp;
    VkPipelineMultisampleStateCreateInfo ms;
    VkDynamicState dynamicStateEnables[2];
    VkPipelineDynamicStateCreateInfo dynamicState;

    VkResult U_ASSERT_ONLY err;

    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
    memset(&dynamicState, 0, sizeof dynamicState);
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables;

    memset(&pipeline, 0, sizeof(pipeline));
    pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline.layout = demo->pipeline_layout;

    vi = demo->vertices.vi;

    memset(&ia, 0, sizeof(ia));
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    memset(&rs, 0, sizeof(rs));
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs.depthClampEnable = VK_FALSE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.depthBiasEnable = VK_FALSE;
    rs.lineWidth = 1.0f;

    memset(&cb, 0, sizeof(cb));
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    VkPipelineColorBlendAttachmentState att_state[1];
    memset(att_state, 0, sizeof(att_state));
    att_state[0].colorWriteMask = 0xf;
    att_state[0].blendEnable = VK_FALSE;
    cb.attachmentCount = 1;
    cb.pAttachments = att_state;

    memset(&vp, 0, sizeof(vp));
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    dynamicStateEnables[dynamicState.dynamicStateCount++] =
        VK_DYNAMIC_STATE_VIEWPORT;
    vp.scissorCount = 1;
    dynamicStateEnables[dynamicState.dynamicStateCount++] =
        VK_DYNAMIC_STATE_SCISSOR;

    memset(&ds, 0, sizeof(ds));
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.back.failOp = VK_STENCIL_OP_KEEP;
    ds.back.passOp = VK_STENCIL_OP_KEEP;
    ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
    ds.stencilTestEnable = VK_FALSE;
    ds.front = ds.back;

    memset(&ms, 0, sizeof(ms));
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.pSampleMask = NULL;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Two stages: vs and fs
    pipeline.stageCount = 2;
    VkPipelineShaderStageCreateInfo shaderStages[2];
    memset(&shaderStages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = demo_prepare_vs(demo);
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = demo_prepare_fs(demo);
    shaderStages[1].pName = "main";

    pipeline.pVertexInputState = &vi;
    pipeline.pInputAssemblyState = &ia;
    pipeline.pRasterizationState = &rs;
    pipeline.pColorBlendState = &cb;
    pipeline.pMultisampleState = &ms;
    pipeline.pViewportState = &vp;
    pipeline.pDepthStencilState = &ds;
    pipeline.pStages = shaderStages;
    pipeline.renderPass = demo->render_pass;
    pipeline.pDynamicState = &dynamicState;

    memset(&pipelineCache, 0, sizeof(pipelineCache));
    pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    err = vkCreatePipelineCache(demo->device, &pipelineCache, NULL,
                                &demo->pipelineCache);
    assert(!err);
    err = vkCreateGraphicsPipelines(demo->device, demo->pipelineCache, 1,
                                    &pipeline, NULL, &demo->pipeline);
    assert(!err);

    vkDestroyPipelineCache(demo->device, demo->pipelineCache, NULL);

    vkDestroyShaderModule(demo->device, demo->frag_shader_module, NULL);
    vkDestroyShaderModule(demo->device, demo->vert_shader_module, NULL);
}

static void demo_prepare_descriptor_pool(struct demo *demo) {
    const VkDescriptorPoolSize type_count = {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = DEMO_TEXTURE_COUNT,
    };
    const VkDescriptorPoolCreateInfo descriptor_pool = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &type_count,
    };
    VkResult U_ASSERT_ONLY err;

    err = vkCreateDescriptorPool(demo->device, &descriptor_pool, NULL,
                                 &demo->desc_pool);
    assert(!err);
}

static void demo_prepare_descriptor_set(struct demo *demo) {
    VkDescriptorImageInfo tex_descs[DEMO_TEXTURE_COUNT];
    VkWriteDescriptorSet write;
    VkResult U_ASSERT_ONLY err;
    uint32_t i;

    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = NULL,
        .descriptorPool = demo->desc_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &demo->desc_layout};
    err = vkAllocateDescriptorSets(demo->device, &alloc_info, &demo->desc_set);
    assert(!err);

    memset(&tex_descs, 0, sizeof(tex_descs));
    for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        tex_descs[i].sampler = demo->textures[i].sampler;
        tex_descs[i].imageView = demo->textures[i].view;
        tex_descs[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    memset(&write, 0, sizeof(write));
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = demo->desc_set;
    write.descriptorCount = DEMO_TEXTURE_COUNT;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = tex_descs;

    vkUpdateDescriptorSets(demo->device, 1, &write, 0, NULL);
}

static void demo_prepare_framebuffers(struct demo *demo) {
    VkImageView attachments[2];
    attachments[1] = demo->depth.view;

    const VkFramebufferCreateInfo fb_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = NULL,
        .renderPass = demo->render_pass,
        .attachmentCount = 2,
        .pAttachments = attachments,
        .width = demo->width,
        .height = demo->height,
        .layers = 1,
    };
    VkResult U_ASSERT_ONLY err;
    uint32_t i;

    demo->framebuffers = (VkFramebuffer *)malloc(demo->swapchainImageCount *
                                                 sizeof(VkFramebuffer));
    assert(demo->framebuffers);

    for (i = 0; i < demo->swapchainImageCount; i++) {
        attachments[0] = demo->buffers[i].view;
        err = vkCreateFramebuffer(demo->device, &fb_info, NULL,
                                  &demo->framebuffers[i]);
        assert(!err);
    }
}

static void demo_prepare(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;

    const VkCommandPoolCreateInfo cmd_pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = NULL,
        .queueFamilyIndex = demo->graphics_queue_node_index,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    };
    err = vkCreateCommandPool(demo->device, &cmd_pool_info, NULL,
                              &demo->cmd_pool);
    assert(!err);

    const VkCommandBufferAllocateInfo cmd = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = demo->cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    err = vkAllocateCommandBuffers(demo->device, &cmd, &demo->draw_cmd);
    assert(!err);

    demo_prepare_buffers(demo);
    demo_prepare_depth(demo);
    demo_prepare_textures(demo);
    demo_prepare_vertices(demo);
    demo_prepare_descriptor_layout(demo);
    demo_prepare_render_pass(demo);
    demo_prepare_pipeline(demo);

    demo_prepare_descriptor_pool(demo);
    demo_prepare_descriptor_set(demo);

    demo_prepare_framebuffers(demo);
}

static void demo_error_callback(int error, const char* description) {
    printf("GLFW error: %s\n", description);
    fflush(stdout);
}

static void demo_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void demo_refresh_callback(GLFWwindow* window) {
    struct demo* demo = glfwGetWindowUserPointer(window);
    demo_draw(demo);
}

static void demo_resize_callback(GLFWwindow* window, int width, int height) {
    struct demo* demo = glfwGetWindowUserPointer(window);
    demo->width = width;
    demo->height = height;
    demo_resize(demo);
}

static void demo_run(struct demo *demo) {
    while (!glfwWindowShouldClose(demo->window)) {
        glfwPollEvents();

        demo_draw(demo);

        if (demo->depthStencil > 0.99f)
            demo->depthIncrement = -0.001f;
        if (demo->depthStencil < 0.8f)
            demo->depthIncrement = 0.001f;

        demo->depthStencil += demo->depthIncrement;

        // Wait for work to finish before updating MVP.
        vkDeviceWaitIdle(demo->device);
        demo->curFrame++;
        if (demo->frameCount != INT32_MAX && demo->curFrame == demo->frameCount)
            glfwSetWindowShouldClose(demo->window, GLFW_TRUE);
    }
}

static void demo_create_window(struct demo *demo) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    demo->window = glfwCreateWindow(demo->width,
                                    demo->height,
                                    APP_LONG_NAME,
                                    NULL,
                                    NULL);
    if (!demo->window) {
        // It didn't work, so try to give a useful error:
        printf("Cannot create a window in which to draw!\n");
        fflush(stdout);
        exit(1);
    }

    glfwSetWindowUserPointer(demo->window, demo);
    glfwSetWindowRefreshCallback(demo->window, demo_refresh_callback);
    glfwSetFramebufferSizeCallback(demo->window, demo_resize_callback);
    glfwSetKeyCallback(demo->window, demo_key_callback);
}

/*
 * Return 1 (true) if all layer names specified in check_names
 * can be found in given layer properties.
 */
static VkBool32 demo_check_layers(uint32_t check_count, const char **check_names,
                                  uint32_t layer_count,
                                  VkLayerProperties *layers) {
    uint32_t i, j;
    for (i = 0; i < check_count; i++) {
        VkBool32 found = 0;
        for (j = 0; j < layer_count; j++) {
            if (!strcmp(check_names[i], layers[j].layerName)) {
                found = 1;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Cannot find layer: %s\n", check_names[i]);
            return 0;
        }
    }
    return 1;
}

static void demo_init_vk(struct demo *demo) {
    VkResult err;
    VkBool32 portability_enumeration = VK_FALSE;
    uint32_t i = 0;
    uint32_t required_extension_count = 0;
    uint32_t instance_extension_count = 0;
    uint32_t instance_layer_count = 0;
    uint32_t validation_layer_count = 0;
    const char **required_extensions = NULL;
    const char **instance_validation_layers = NULL;
    demo->enabled_extension_count = 0;
    demo->enabled_layer_count = 0;

    char *instance_validation_layers_alt1[] = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    char *instance_validation_layers_alt2[] = {
        "VK_LAYER_GOOGLE_threading",       "VK_LAYER_LUNARG_parameter_validation",
        "VK_LAYER_LUNARG_object_tracker",  "VK_LAYER_LUNARG_image",
        "VK_LAYER_LUNARG_core_validation", "VK_LAYER_LUNARG_swapchain",
        "VK_LAYER_GOOGLE_unique_objects"
    };

    /* Look for validation layers */
    VkBool32 validation_found = 0;
    if (demo->validate) {

        err = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
        assert(!err);

        instance_validation_layers = (const char**) instance_validation_layers_alt1;
        if (instance_layer_count > 0) {
            VkLayerProperties *instance_layers =
                    malloc(sizeof (VkLayerProperties) * instance_layer_count);
            err = vkEnumerateInstanceLayerProperties(&instance_layer_count,
                    instance_layers);
            assert(!err);


            validation_found = demo_check_layers(
                    ARRAY_SIZE(instance_validation_layers_alt1),
                    instance_validation_layers, instance_layer_count,
                    instance_layers);
            if (validation_found) {
                demo->enabled_layer_count = ARRAY_SIZE(instance_validation_layers_alt1);
                demo->enabled_layers[0] = "VK_LAYER_LUNARG_standard_validation";
                validation_layer_count = 1;
            } else {
                // use alternative set of validation layers
                instance_validation_layers =
                    (const char**) instance_validation_layers_alt2;
                demo->enabled_layer_count = ARRAY_SIZE(instance_validation_layers_alt2);
                validation_found = demo_check_layers(
                    ARRAY_SIZE(instance_validation_layers_alt2),
                    instance_validation_layers, instance_layer_count,
                    instance_layers);
                validation_layer_count =
                    ARRAY_SIZE(instance_validation_layers_alt2);
                for (i = 0; i < validation_layer_count; i++) {
                    demo->enabled_layers[i] = instance_validation_layers[i];
                }
            }
            free(instance_layers);
        }

        if (!validation_found) {
            ERR_EXIT("vkEnumerateInstanceLayerProperties failed to find "
                    "required validation layer.\n\n"
                    "Please look at the Getting Started guide for additional "
                    "information.\n",
                    "vkCreateInstance Failure");
        }
    }

    /* Look for instance extensions */
    required_extensions = glfwGetRequiredInstanceExtensions(&required_extension_count);
    if (!required_extensions) {
        ERR_EXIT("glfwGetRequiredInstanceExtensions failed to find the "
                 "platform surface extensions.\n\nDo you have a compatible "
                 "Vulkan installable client driver (ICD) installed?\nPlease "
                 "look at the Getting Started guide for additional "
                 "information.\n",
                 "vkCreateInstance Failure");
    }

    for (i = 0; i < required_extension_count; i++) {
        demo->extension_names[demo->enabled_extension_count++] = required_extensions[i];
        assert(demo->enabled_extension_count < 64);
    }

    err = vkEnumerateInstanceExtensionProperties(
        NULL, &instance_extension_count, NULL);
    assert(!err);

    if (instance_extension_count > 0) {
        VkExtensionProperties *instance_extensions =
            malloc(sizeof(VkExtensionProperties) * instance_extension_count);
        err = vkEnumerateInstanceExtensionProperties(
            NULL, &instance_extension_count, instance_extensions);
        assert(!err);
        for (i = 0; i < instance_extension_count; i++) {
            if (!strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                        instance_extensions[i].extensionName)) {
                if (demo->validate) {
                    demo->extension_names[demo->enabled_extension_count++] =
                        VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
                }
            }
            assert(demo->enabled_extension_count < 64);
            if (!strcmp(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
                        instance_extensions[i].extensionName)) {
                demo->extension_names[demo->enabled_extension_count++] =
                    VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
                portability_enumeration = VK_TRUE;
            }
            assert(demo->enabled_extension_count < 64);
        }

        free(instance_extensions);
    }

    const VkApplicationInfo app = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = APP_SHORT_NAME,
        .applicationVersion = 0,
        .pEngineName = APP_SHORT_NAME,
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_0,
    };
    VkInstanceCreateInfo inst_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .pApplicationInfo = &app,
        .enabledLayerCount = demo->enabled_layer_count,
        .ppEnabledLayerNames = (const char *const *)instance_validation_layers,
        .enabledExtensionCount = demo->enabled_extension_count,
        .ppEnabledExtensionNames = (const char *const *)demo->extension_names,
    };

    if (portability_enumeration)
        inst_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    uint32_t gpu_count;

    err = vkCreateInstance(&inst_info, NULL, &demo->inst);
    if (err == VK_ERROR_INCOMPATIBLE_DRIVER) {
        ERR_EXIT("Cannot find a compatible Vulkan installable client driver "
                 "(ICD).\n\nPlease look at the Getting Started guide for "
                 "additional information.\n",
                 "vkCreateInstance Failure");
    } else if (err == VK_ERROR_EXTENSION_NOT_PRESENT) {
        ERR_EXIT("Cannot find a specified extension library"
                 ".\nMake sure your layers path is set appropriately\n",
                 "vkCreateInstance Failure");
    } else if (err) {
        ERR_EXIT("vkCreateInstance failed.\n\nDo you have a compatible Vulkan "
                 "installable client driver (ICD) installed?\nPlease look at "
                 "the Getting Started guide for additional information.\n",
                 "vkCreateInstance Failure");
    }

    gladLoadVulkanUserPtr(NULL, (GLADuserptrloadfunc) glfwGetInstanceProcAddress, demo->inst);

    /* Make initial call to query gpu_count, then second call for gpu info*/
    err = vkEnumeratePhysicalDevices(demo->inst, &gpu_count, NULL);
    assert(!err && gpu_count > 0);

    if (gpu_count > 0) {
        VkPhysicalDevice *physical_devices =
            malloc(sizeof(VkPhysicalDevice) * gpu_count);
        err = vkEnumeratePhysicalDevices(demo->inst, &gpu_count,
                                         physical_devices);
        assert(!err);
        /* For tri demo we just grab the first physical device */
        demo->gpu = physical_devices[0];
        free(physical_devices);
    } else {
        ERR_EXIT("vkEnumeratePhysicalDevices reported zero accessible devices."
                 "\n\nDo you have a compatible Vulkan installable client"
                 " driver (ICD) installed?\nPlease look at the Getting Started"
                 " guide for additional information.\n",
                 "vkEnumeratePhysicalDevices Failure");
    }

    gladLoadVulkanUserPtr(demo->gpu, (GLADuserptrloadfunc) glfwGetInstanceProcAddress, demo->inst);

    /* Look for device extensions */
    uint32_t device_extension_count = 0;
    VkBool32 swapchainExtFound = 0;
    demo->enabled_extension_count = 0;

    err = vkEnumerateDeviceExtensionProperties(demo->gpu, NULL,
                                               &device_extension_count, NULL);
    assert(!err);

    if (device_extension_count > 0) {
        VkExtensionProperties *device_extensions =
                malloc(sizeof(VkExtensionProperties) * device_extension_count);
        err = vkEnumerateDeviceExtensionProperties(
            demo->gpu, NULL, &device_extension_count, device_extensions);
        assert(!err);

        for (i = 0; i < device_extension_count; i++) {
            if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                        device_extensions[i].extensionName)) {
                swapchainExtFound = 1;
                demo->extension_names[demo->enabled_extension_count++] =
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME;
            }
            assert(demo->enabled_extension_count < 64);
        }

        free(device_extensions);
    }

    if (!swapchainExtFound) {
        ERR_EXIT("vkEnumerateDeviceExtensionProperties failed to find "
                 "the " VK_KHR_SWAPCHAIN_EXTENSION_NAME
                 " extension.\n\nDo you have a compatible "
                 "Vulkan installable client driver (ICD) installed?\nPlease "
                 "look at the Getting Started guide for additional "
                 "information.\n",
                 "vkCreateInstance Failure");
    }

    if (demo->validate) {
        VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
        dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        dbgCreateInfo.flags =
            VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        dbgCreateInfo.pfnCallback = demo->use_break ? BreakCallback : dbgFunc;
        dbgCreateInfo.pUserData = demo;
        dbgCreateInfo.pNext = NULL;
        err = vkCreateDebugReportCallbackEXT(demo->inst, &dbgCreateInfo, NULL,
                                             &demo->msg_callback);
        switch (err) {
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            ERR_EXIT("CreateDebugReportCallback: out of host memory\n",
                     "CreateDebugReportCallback Failure");
            break;
        default:
            ERR_EXIT("CreateDebugReportCallback: unknown failure\n",
                     "CreateDebugReportCallback Failure");
            break;
        }
    }

    vkGetPhysicalDeviceProperties(demo->gpu, &demo->gpu_props);

    // Query with NULL data to get count
    vkGetPhysicalDeviceQueueFamilyProperties(demo->gpu, &demo->queue_count,
                                             NULL);

    demo->queue_props = (VkQueueFamilyProperties *)malloc(
        demo->queue_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(demo->gpu, &demo->queue_count,
                                             demo->queue_props);
    assert(demo->queue_count >= 1);

    vkGetPhysicalDeviceFeatures(demo->gpu, &demo->gpu_features);

    // Graphics queue and MemMgr queue can be separate.
    // TODO: Add support for separate queues, including synchronization,
    //       and appropriate tracking for QueueSubmit
}

static void demo_init_device(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;

    float queue_priorities[1] = {0.0};
    const VkDeviceQueueCreateInfo queue = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = NULL,
        .queueFamilyIndex = demo->graphics_queue_node_index,
        .queueCount = 1,
        .pQueuePriorities = queue_priorities};


    VkPhysicalDeviceFeatures features;
    memset(&features, 0, sizeof(features));
    if (demo->gpu_features.shaderClipDistance) {
        features.shaderClipDistance = VK_TRUE;
    }

    VkDeviceCreateInfo device = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = NULL,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = demo->enabled_extension_count,
        .ppEnabledExtensionNames = (const char *const *)demo->extension_names,
        .pEnabledFeatures = &features,
    };

    err = vkCreateDevice(demo->gpu, &device, NULL, &demo->device);
    assert(!err);
}

static void demo_init_vk_swapchain(struct demo *demo) {
    VkResult U_ASSERT_ONLY err;
    uint32_t i;

    // Create a WSI surface for the window:
    glfwCreateWindowSurface(demo->inst, demo->window, NULL, &demo->surface);

    // Iterate over each queue to learn whether it supports presenting:
    VkBool32 *supportsPresent =
        (VkBool32 *)malloc(demo->queue_count * sizeof(VkBool32));
    for (i = 0; i < demo->queue_count; i++) {
        vkGetPhysicalDeviceSurfaceSupportKHR(demo->gpu, i, demo->surface,
                                             &supportsPresent[i]);
    }

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    uint32_t graphicsQueueNodeIndex = UINT32_MAX;
    uint32_t presentQueueNodeIndex = UINT32_MAX;
    for (i = 0; i < demo->queue_count; i++) {
        if ((demo->queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            if (graphicsQueueNodeIndex == UINT32_MAX) {
                graphicsQueueNodeIndex = i;
            }

            if (supportsPresent[i] == VK_TRUE) {
                graphicsQueueNodeIndex = i;
                presentQueueNodeIndex = i;
                break;
            }
        }
    }
    if (presentQueueNodeIndex == UINT32_MAX) {
        // If didn't find a queue that supports both graphics and present, then
        // find a separate present queue.
        for (i = 0; i < demo->queue_count; ++i) {
            if (supportsPresent[i] == VK_TRUE) {
                presentQueueNodeIndex = i;
                break;
            }
        }
    }
    free(supportsPresent);

    // Generate error if could not find both a graphics and a present queue
    if (graphicsQueueNodeIndex == UINT32_MAX ||
        presentQueueNodeIndex == UINT32_MAX) {
        ERR_EXIT("Could not find a graphics and a present queue\n",
                 "Swapchain Initialization Failure");
    }

    // TODO: Add support for separate queues, including presentation,
    //       synchronization, and appropriate tracking for QueueSubmit.
    // NOTE: While it is possible for an application to use a separate graphics
    //       and a present queues, this demo program assumes it is only using
    //       one:
    if (graphicsQueueNodeIndex != presentQueueNodeIndex) {
        ERR_EXIT("Could not find a common graphics and a present queue\n",
                 "Swapchain Initialization Failure");
    }

    demo->graphics_queue_node_index = graphicsQueueNodeIndex;

    demo_init_device(demo);

    vkGetDeviceQueue(demo->device, demo->graphics_queue_node_index, 0,
                     &demo->queue);

    // Get the list of VkFormat's that are supported:
    uint32_t formatCount;
    err = vkGetPhysicalDeviceSurfaceFormatsKHR(demo->gpu, demo->surface,
                                               &formatCount, NULL);
    assert(!err);
    VkSurfaceFormatKHR *surfFormats =
        (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    err = vkGetPhysicalDeviceSurfaceFormatsKHR(demo->gpu, demo->surface,
                                               &formatCount, surfFormats);
    assert(!err);
    // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
    // the surface has no preferred format.  Otherwise, at least one
    // supported format will be returned.
    if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
        demo->format = VK_FORMAT_B8G8R8A8_UNORM;
    } else {
        assert(formatCount >= 1);
        demo->format = surfFormats[0].format;
    }
    demo->color_space = surfFormats[0].colorSpace;

    demo->curFrame = 0;

    // Get Memory information and properties
    vkGetPhysicalDeviceMemoryProperties(demo->gpu, &demo->memory_properties);
}

static void demo_init_connection(struct demo *demo) {
    glfwSetErrorCallback(demo_error_callback);

    if (!glfwInit()) {
        printf("Cannot initialize GLFW.\nExiting ...\n");
        fflush(stdout);
        exit(1);
    }

    if (!glfwVulkanSupported()) {
        printf("GLFW failed to find the Vulkan loader.\nExiting ...\n");
        fflush(stdout);
        exit(1);
    }

    gladLoadVulkanUserPtr(NULL, (GLADuserptrloadfunc) glfwGetInstanceProcAddress, NULL);
}

static void demo_init(struct demo *demo, const int argc, const char *argv[])
{
    int i;
    memset(demo, 0, sizeof(*demo));
    demo->frameCount = INT32_MAX;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--use_staging") == 0) {
            demo->use_staging_buffer = true;
            continue;
        }
        if (strcmp(argv[i], "--break") == 0) {
            demo->use_break = true;
            continue;
        }
        if (strcmp(argv[i], "--validate") == 0) {
            demo->validate = true;
            continue;
        }
        if (strcmp(argv[i], "--c") == 0 && demo->frameCount == INT32_MAX &&
            i < argc - 1 && sscanf(argv[i + 1], "%d", &demo->frameCount) == 1 &&
            demo->frameCount >= 0) {
            i++;
            continue;
        }

        fprintf(stderr, "Usage:\n  %s [--use_staging] [--validate] [--break] "
                        "[--c <framecount>]\n",
                APP_SHORT_NAME);
        fflush(stderr);
        exit(1);
    }

    demo_init_connection(demo);
    demo_init_vk(demo);

    demo->width = 300;
    demo->height = 300;
    demo->depthStencil = 1.0;
    demo->depthIncrement = -0.01f;
}

static void demo_cleanup(struct demo *demo) {
    uint32_t i;

    for (i = 0; i < demo->swapchainImageCount; i++) {
        vkDestroyFramebuffer(demo->device, demo->framebuffers[i], NULL);
    }
    free(demo->framebuffers);
    vkDestroyDescriptorPool(demo->device, demo->desc_pool, NULL);

    if (demo->setup_cmd) {
        vkFreeCommandBuffers(demo->device, demo->cmd_pool, 1, &demo->setup_cmd);
    }
    vkFreeCommandBuffers(demo->device, demo->cmd_pool, 1, &demo->draw_cmd);
    vkDestroyCommandPool(demo->device, demo->cmd_pool, NULL);

    vkDestroyPipeline(demo->device, demo->pipeline, NULL);
    vkDestroyRenderPass(demo->device, demo->render_pass, NULL);
    vkDestroyPipelineLayout(demo->device, demo->pipeline_layout, NULL);
    vkDestroyDescriptorSetLayout(demo->device, demo->desc_layout, NULL);

    vkDestroyBuffer(demo->device, demo->vertices.buf, NULL);
    vkFreeMemory(demo->device, demo->vertices.mem, NULL);

    for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        vkDestroyImageView(demo->device, demo->textures[i].view, NULL);
        vkDestroyImage(demo->device, demo->textures[i].image, NULL);
        vkFreeMemory(demo->device, demo->textures[i].mem, NULL);
        vkDestroySampler(demo->device, demo->textures[i].sampler, NULL);
    }

    for (i = 0; i < demo->swapchainImageCount; i++) {
        vkDestroyImageView(demo->device, demo->buffers[i].view, NULL);
    }

    vkDestroyImageView(demo->device, demo->depth.view, NULL);
    vkDestroyImage(demo->device, demo->depth.image, NULL);
    vkFreeMemory(demo->device, demo->depth.mem, NULL);

    vkDestroySwapchainKHR(demo->device, demo->swapchain, NULL);
    free(demo->buffers);

    vkDestroyDevice(demo->device, NULL);
    if (demo->validate) {
        vkDestroyDebugReportCallbackEXT(demo->inst, demo->msg_callback, NULL);
    }
    vkDestroySurfaceKHR(demo->inst, demo->surface, NULL);
    vkDestroyInstance(demo->inst, NULL);

    free(demo->queue_props);

    glfwDestroyWindow(demo->window);
    glfwTerminate();
}

static void demo_resize(struct demo *demo) {
    uint32_t i;

    // In order to properly resize the window, we must re-create the swapchain
    // AND redo the command buffers, etc.
    //
    // First, perform part of the demo_cleanup() function:

    for (i = 0; i < demo->swapchainImageCount; i++) {
        vkDestroyFramebuffer(demo->device, demo->framebuffers[i], NULL);
    }
    free(demo->framebuffers);
    vkDestroyDescriptorPool(demo->device, demo->desc_pool, NULL);

    if (demo->setup_cmd) {
        vkFreeCommandBuffers(demo->device, demo->cmd_pool, 1, &demo->setup_cmd);
        demo->setup_cmd = VK_NULL_HANDLE;
    }
    vkFreeCommandBuffers(demo->device, demo->cmd_pool, 1, &demo->draw_cmd);
    vkDestroyCommandPool(demo->device, demo->cmd_pool, NULL);

    vkDestroyPipeline(demo->device, demo->pipeline, NULL);
    vkDestroyRenderPass(demo->device, demo->render_pass, NULL);
    vkDestroyPipelineLayout(demo->device, demo->pipeline_layout, NULL);
    vkDestroyDescriptorSetLayout(demo->device, demo->desc_layout, NULL);

    vkDestroyBuffer(demo->device, demo->vertices.buf, NULL);
    vkFreeMemory(demo->device, demo->vertices.mem, NULL);

    for (i = 0; i < DEMO_TEXTURE_COUNT; i++) {
        vkDestroyImageView(demo->device, demo->textures[i].view, NULL);
        vkDestroyImage(demo->device, demo->textures[i].image, NULL);
        vkFreeMemory(demo->device, demo->textures[i].mem, NULL);
        vkDestroySampler(demo->device, demo->textures[i].sampler, NULL);
    }

    for (i = 0; i < demo->swapchainImageCount; i++) {
        vkDestroyImageView(demo->device, demo->buffers[i].view, NULL);
    }

    vkDestroyImageView(demo->device, demo->depth.view, NULL);
    vkDestroyImage(demo->device, demo->depth.image, NULL);
    vkFreeMemory(demo->device, demo->depth.mem, NULL);

    free(demo->buffers);

    // Second, re-perform the demo_prepare() function, which will re-create the
    // swapchain:
    demo_prepare(demo);
}

int main(const int argc, const char *argv[]) {
    struct demo demo;

    demo_init(&demo, argc, argv);
    demo_create_window(&demo);
    demo_init_vk_swapchain(&demo);

    demo_prepare(&demo);
    demo_run(&demo);

    demo_cleanup(&demo);

    return validation_error;
}

