#ifndef VULKAN_COMPUTE_PIPELINE_H
#define VULKAN_COMPUTE_PIPELINE_H

#include <vulkan/vulkan.h>

void create_compute_pipeline(
    VkDevice logical_device, 
    VkDescriptorSetLayout descriptor_set_layout, 
    VkPipelineLayout& compute_pipeline_layout, 
    VkPipeline& compute_pipeline
);

#endif