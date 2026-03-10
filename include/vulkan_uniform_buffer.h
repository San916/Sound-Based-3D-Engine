#ifndef VULKAN_UNIFORM_BUFFER_H
#define VULKAN_UNIFORM_BUFFER_H

#include <iostream>

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

typedef struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 position;
} UniformBufferObject;

void create_uniform_buffers(
    VkDevice logical_device,
    VkPhysicalDevice physical_device,
    size_t max_frames_in_flight,
    std::vector<VkBuffer>& uniform_buffers,
    std::vector<VkDeviceMemory>& uniform_buffers_memory,
    std::vector<void*>& uniform_buffers_mapped
);
void update_uniform_buffer(uint32_t frame_index, std::vector<void*>& uniform_buffers_mapped);

#endif