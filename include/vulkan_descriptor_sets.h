#ifndef VULKAN_DESCRIPTOR_SETS_H
#define VULKAN_DESCRIPTOR_SETS_H

#include <vector>

#include <vulkan/vulkan.h>

void create_graphics_descriptor_set_layout(VkDevice logical_device, VkDescriptorSetLayout& graphics_descriptor_set_layout);
void create_compute_descriptor_set_layout(VkDevice logical_device, VkDescriptorSetLayout& compute_descriptor_set_layout);
void create_graphics_descriptor_pool(VkDevice logical_device, size_t max_frames_in_flight, VkDescriptorPool& graphics_descriptor_pool);
void create_compute_descriptor_pool(VkDevice logical_device, size_t max_frames_in_flight, VkDescriptorPool& compute_descriptor_pool);
void create_graphics_descriptor_sets(
    VkDevice logical_device, 
    size_t max_frames_in_flight, 
    VkDescriptorPool graphics_descriptor_pool, 
    const std::vector<VkBuffer>& uniform_buffers, 
    VkSampler& storage_image_sampler, 
    const VkImageView storage_image_view, 
    const VkDescriptorSetLayout& graphics_descriptor_set_layout, 
    std::vector<VkDescriptorSet>& graphics_descriptor_sets
);
void create_compute_descriptor_sets(
    VkDevice logical_device, 
    size_t max_frames_in_flight, 
    VkDescriptorPool compute_descriptor_pool, 
    const VkAccelerationStructureKHR& tlas, 
    const VkImageView storage_image_view, 
    const VkDescriptorSetLayout& compute_descriptor_set_layout, 
    std::vector<VkDescriptorSet>& compute_descriptor_sets
);

#endif