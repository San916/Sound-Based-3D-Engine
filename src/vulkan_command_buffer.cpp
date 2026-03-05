#include <iostream>
#include <cstdint>

#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan_command_buffer.h>

// MODIFIES: command_pool
// EFFECTS: Creates a command pool and returns it
void create_command_pool(VkDevice logical_device, uint32_t graphics_queue_family_index, VkCommandPool& command_pool) {
    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = nullptr;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = graphics_queue_family_index;

    if (vkCreateCommandPool(logical_device, &command_pool_create_info, nullptr, &command_pool) != VK_SUCCESS) {
        throw std::runtime_error("create_command_pool(): Failed to create command pool!");
    }
}

// MODIFIES: command_buffers
// EFFECTS: Allocates command buffers with num_command_buffers and returns it
void create_command_buffers(VkDevice logical_device, VkCommandPool command_pool, std::vector<VkCommandBuffer>& command_buffers, size_t num_command_buffers) {
    command_buffers.resize(num_command_buffers);

    VkCommandBufferAllocateInfo command_buffers_create_info{};
    command_buffers_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffers_create_info.pNext = nullptr;
    command_buffers_create_info.commandPool = command_pool;
    command_buffers_create_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffers_create_info.commandBufferCount = (uint32_t)num_command_buffers;

    if (vkAllocateCommandBuffers(logical_device, &command_buffers_create_info, command_buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("create_command_buffers(): Failed to alloccate command buffers!");
    }
}

// REQUIRES: swap_chain_image_index < frame_buffers.size()
// MODIFIES: frame_buffers, graphics_pipeline, command_buffer
// EFFECTS: Records into command_buffer the command to draw into a frame buffer
// Inserts operation into graphics pipeline
void draw_command_buffer(
    VkRenderPass render_pass, std::vector<VkFramebuffer>& frame_buffers, 
    VkExtent2D swap_chain_extent, size_t swap_chain_image_index, size_t frame_index,
    VkPipelineLayout pipeline_layout, const std::vector<VkDescriptorSet>& descriptor_sets,
    VkPipeline& graphics_pipeline, VkCommandBuffer& command_buffer
) {
    if (swap_chain_image_index >= frame_buffers.size()) {
        throw std::runtime_error("draw_command_buffer(): Swap chain image index is not less than frame buffer size!");
    }

    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.pNext = nullptr;
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info) != VK_SUCCESS) {
        throw std::runtime_error("draw_command_buffer(): Failed to begin writing command buffer!");
    }

    VkRenderPassBeginInfo render_pass_begin_info{};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.pNext = nullptr;
    render_pass_begin_info.renderPass = render_pass;
    render_pass_begin_info.framebuffer = frame_buffers[swap_chain_image_index];
    render_pass_begin_info.renderArea.offset = {0, 0};
    render_pass_begin_info.renderArea.extent = swap_chain_extent;

    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

    // Drawing should happen here    
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_sets[frame_index], 0, nullptr);

    vkCmdDraw(command_buffer, 3, 1, 0, 0); // Placeholder
    //

    vkCmdEndRenderPass(command_buffer);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("draw_command_buffer(): Failed to write command buffer!");
    }
}