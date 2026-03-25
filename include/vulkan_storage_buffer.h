#ifndef VULKAN_STORAGE_BUFFER_H
#define VULKAN_STORAGE_BUFFER_H

#define MAX_SOUND_WAVES 1024
#define MAX_OBJECTS 16

#include <iostream>
#include <cstdint>

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

typedef struct ObjectProperties ObjectProperties;

struct SoundWave {
    glm::vec4 data;
    float amplitude;
};

struct StorageBufferObject {
    glm::mat4 model[MAX_OBJECTS];
    glm::vec4 sound_waves[MAX_SOUND_WAVES];
    float amplitudes[MAX_SOUND_WAVES];
    int visible[MAX_OBJECTS];
    int emitting[MAX_OBJECTS];
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
    const std::vector<ObjectProperties> properties,
    const std::vector<SoundWave>& sound_waves,
    std::vector<void*>& storage_buffers_mapped
);

#endif