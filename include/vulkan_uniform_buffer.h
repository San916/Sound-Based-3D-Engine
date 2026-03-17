#ifndef VULKAN_UNIFORM_BUFFER_H
#define VULKAN_UNIFORM_BUFFER_H

#include <iostream>

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#define MAX_SOUND_WAVES 16
#define MAX_OBJECTS 16

typedef struct UniformBufferObject {
    glm::mat4 model[MAX_OBJECTS];
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec4 position;
    glm::vec4 sound_waves[MAX_SOUND_WAVES];
} UniformBufferObject;

void create_uniform_buffers(
    VkDevice logical_device,
    VkPhysicalDevice physical_device,
    size_t max_frames_in_flight,
    std::vector<VkBuffer>& uniform_buffers,
    std::vector<VkDeviceMemory>& uniform_buffers_memory,
    std::vector<void*>& uniform_buffers_mapped
);
void update_uniform_buffer(
    uint32_t frame_index,
    VkExtent2D swap_chain_extent,
    const std::vector<glm::mat4>& transforms,
    glm::vec3 camera_position,
    glm::vec2 camera_rotation,
    const std::vector<glm::vec4>& sound_waves,
    std::vector<void*>& uniform_buffers_mapped
);

#endif