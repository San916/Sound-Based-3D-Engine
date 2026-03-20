#include <iostream>
#include <cstdint>

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vulkan_storage_buffer.h>
#include <vulkan_utils.h>

// MODIFIES: storage_buffers, storage_buffers_memory, storage_buffers_mapped
// EFFECTS: Creates storage buffer handles, storage buffer memory, and storage buffer maps
void create_storage_buffers(
    VkDevice logical_device,
    VkPhysicalDevice physical_device,
    size_t max_frames_in_flight,
    std::vector<VkBuffer>& storage_buffers,
    std::vector<VkDeviceMemory>& storage_buffers_memory,
    std::vector<void*>& storage_buffers_mapped
) {
    VkDeviceSize storage_buffer_size = sizeof(StorageBufferObject);

    storage_buffers.resize(max_frames_in_flight);
    storage_buffers_memory.resize(max_frames_in_flight);
    storage_buffers_mapped.resize(max_frames_in_flight);

    for (size_t i = 0; i < max_frames_in_flight; i++) {
        create_buffer(
            logical_device,
            physical_device,
            storage_buffer_size, 
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            storage_buffers[i], 
            storage_buffers_memory[i]
        );

        vkMapMemory(logical_device, storage_buffers_memory[i], 0, storage_buffer_size, 0, &storage_buffers_mapped[i]);
    }
}

// MODIFIES: storage_buffers_mapped
// EFFECTS: Copies the current storage buffer to the storage buffers mapped memory
//     Sets the model matrices for each object, as well as the propagating sound waves
void update_storage_buffer(
    uint32_t frame_index,
    int selected_object_index,
    const std::vector<glm::mat4>& transforms,
    const std::vector<glm::vec4>& sound_waves,
    std::vector<void*>& storage_buffers_mapped
) {
    StorageBufferObject storage_buffer{};

    storage_buffer.selected_object_index = selected_object_index;

    for (size_t i = 0; i < transforms.size() && i < MAX_OBJECTS; i++) {
        storage_buffer.model[i] = transforms[i];
    }

    for (size_t i = 0; i < MAX_SOUND_WAVES; i++) {
        storage_buffer.sound_waves[i] = (i < sound_waves.size()) ? sound_waves[i] : glm::vec4(0.0f, 0.0f, 0.0f, -1.0f);
    }

    memcpy(storage_buffers_mapped[frame_index], &storage_buffer, sizeof(storage_buffer));
}