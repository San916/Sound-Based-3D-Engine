#ifndef VULKAN_COMMAND_BUFFER_H
#define VULKAN_COMMAND_BUFFER_H

#include <cstdint>

#include <vector>

#include <vulkan/vulkan.h>

void create_command_pool(VkDevice logical_device, uint32_t graphics_queue_family_index, VkCommandPool& command_pool);
void create_command_buffers(VkDevice logical_device, VkCommandPool command_pool, std::vector<VkCommandBuffer>& command_buffers, size_t num_command_buffers);
void draw_command_buffer(
    VkRenderPass render_pass, const std::vector<VkFramebuffer>& frame_buffers, 
    VkExtent2D swap_chain_extent, size_t swap_chain_image_index, size_t frame_index,
    VkPipelineLayout pipeline_layout, const std::vector<VkDescriptorSet>& descriptor_sets,
    VkBuffer vertex_buffer, VkBuffer index_buffer, const std::vector<uint32_t>& indices, 
    VkPipeline& graphics_pipeline, VkCommandBuffer& command_buffer
);
void copy_buffer(VkDevice logical_device, VkCommandPool command_pool, VkQueue graphics_queue, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
void begin_single_time_command(VkDevice logical_device, VkCommandPool command_pool, VkCommandBuffer& command_buffer);
void finish_single_time_command(VkDevice logical_device, VkQueue graphics_queue, VkCommandPool command_pool, VkCommandBuffer& command_buffer);

#endif