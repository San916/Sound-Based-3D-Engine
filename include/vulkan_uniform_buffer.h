#ifndef VULKAN_UNIFORM_BUFFER_H
#define VULKAN_UNIFORM_BUFFER_H

#include <iostream>

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

typedef struct CameraUBO {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 position;
} CameraUBO;

void create_descriptor_set_layout(VkDevice logical_device, VkDescriptorSetLayout& descriptor_set_layout);
void create_descriptor_pool(VkDevice logical_device, size_t max_frames_in_flight, VkDescriptorPool& descriptor_pool);
void create_descriptor_sets(
    VkDevice logical_device, 
    size_t max_frames_in_flight, 
    VkDescriptorPool descriptor_pool,
    const VkDescriptorSetLayout& descriptor_set_layout,
    std::vector<VkDescriptorSet>& descriptor_sets
);
void create_uniform_buffers(
    VkDevice logical_device,
    VkPhysicalDevice physical_device,
    size_t max_frames_in_flight,
    std::vector<VkBuffer>& uniform_buffers,
    std::vector<VkDeviceMemory>& uniform_buffers_memory,
    std::vector<void*>& uniform_buffers_mapped
);

#endif