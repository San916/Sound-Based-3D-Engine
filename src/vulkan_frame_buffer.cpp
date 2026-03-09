#include <iostream>

#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan_frame_buffer.h>

// MODIFIES: frame_buffers
// EFFECTS: Creates a frame buffer for each swap chain image view
void create_frame_buffers(
    VkDevice logical_device, const std::vector<VkImageView>& swap_chain_image_views, 
    VkExtent2D swap_chain_extent, VkRenderPass render_pass, std::vector<VkFramebuffer>& frame_buffers
) {
    frame_buffers.resize(swap_chain_image_views.size());

    for (size_t i = 0; i < swap_chain_image_views.size(); i++) {
        VkImageView attachments[] = {
            swap_chain_image_views[i]
        };

        VkFramebufferCreateInfo frame_buffer_create_info{};
        frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frame_buffer_create_info.pNext = nullptr;
        frame_buffer_create_info.renderPass = render_pass;
        frame_buffer_create_info.attachmentCount = 1;
        frame_buffer_create_info.pAttachments = attachments;
        frame_buffer_create_info.width = swap_chain_extent.width;
        frame_buffer_create_info.height = swap_chain_extent.height;
        frame_buffer_create_info.layers = 1;

        if (vkCreateFramebuffer(logical_device, &frame_buffer_create_info, nullptr, &frame_buffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("create_frame_buffers(): Failed to create framebuffer!");
        }
    }
}

