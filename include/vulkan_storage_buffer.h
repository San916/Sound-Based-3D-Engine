#ifndef VULKAN_STORAGE_BUFFER_H
#define VULKAN_STORAGE_BUFFER_H

#define MAX_SOUND_WAVES 16
#define MAX_OBJECTS 16

#include <iostream>
#include <cstdint>

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

struct StorageBufferObject {
    glm::mat4 model[MAX_OBJECTS];
    glm::vec4 sound_waves[MAX_SOUND_WAVES];
    int selected_object_index = -1;
};

void create_storage_buffers(
    VkDevice logical_device,
    VkPhysicalDevice physical_device,
    size_t max_frames_in_flight,
    std::vector<VkBuffer>& storage_buffers,
    std::vector<VkDeviceMemory>& storage_buffers_memory,
    std::vector<void*>& storage_buffers_mapped
);
void update_storage_buffer(
    uint32_t frame_index,
    int selected_object_index,
    const std::vector<glm::mat4>& transforms,
    const std::vector<glm::vec4>& sound_waves,
    std::vector<void*>& storage_buffers_mapped
);

#endif