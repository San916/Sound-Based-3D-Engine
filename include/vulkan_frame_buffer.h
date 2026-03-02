#ifndef VULKAN_FRAME_BUFFER_H
#define VULKAN_FRAME_BUFFER_H

#include <vector>

#include <vulkan/vulkan.h>

void create_frame_buffers(
    VkDevice logical_device, std::vector<VkImageView> swap_chain_image_views, 
    VkExtent2D swap_chain_extent, VkRenderPass render_pass, std::vector<VkFramebuffer>& frame_buffers
);

#endif