#ifndef VULKAN_COMMAND_BUFFER_H
#define VULKAN_COMMAND_BUFFER_H

#include <cstdint>

#include <vulkan/vulkan.h>

void create_command_pool(VkDevice logical_device, uint32_t graphics_queue_family_index, VkCommandPool& command_pool);
void create_command_buffers(VkDevice logical_device, VkCommandPool command_pool, std::vector<VkCommandBuffer>& command_buffers, size_t num_command_buffers);
void draw_command_buffer(
    VkRenderPass render_pass, std::vector<VkFramebuffer>& frame_buffers, 
    VkExtent2D swap_chain_extent, size_t swap_chain_image_index, size_t frame_index,
    VkPipelineLayout pipeline_layout, const std::vector<VkDescriptorSet>& descriptor_sets,
    VkPipeline& graphics_pipeline, VkCommandBuffer& command_buffer
);

#endif