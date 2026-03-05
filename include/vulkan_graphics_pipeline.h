#ifndef VULKAN_GRAPHICS_PIPELINE_H
#define VULKAN_GRAPHICS_PIPELINE_H

#include <vulkan/vulkan.h>

void create_graphics_pipeline(
    VkDevice logical_device,
    VkExtent2D swap_chain_extent, VkFormat swap_chain_image_format,
    VkPipelineLayout& pipeline_layout, VkRenderPass& render_pass, 
    VkDescriptorSetLayout descriptor_set_layout, VkPipeline& graphics_pipeline
);

#endif