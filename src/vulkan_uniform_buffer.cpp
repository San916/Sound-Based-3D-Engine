#include <iostream>

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan_uniform_buffer.h>
#include <vulkan_utils.h>

// MODIFIES: uniform_buffers, uniform_buffers_memory, uniform_buffers_mapped
// EFFECTS: Creates uniform buffer handles, uniform buffer memory, and uniform buffer maps
void create_uniform_buffers(
    VkDevice logical_device,
    VkPhysicalDevice physical_device,
    size_t max_frames_in_flight,
    std::vector<VkBuffer>& uniform_buffers,
    std::vector<VkDeviceMemory>& uniform_buffers_memory,
    std::vector<void*>& uniform_buffers_mapped
) {
    VkDeviceSize uniform_buffer_size = sizeof(UniformBufferObject);

    uniform_buffers.resize(max_frames_in_flight);
    uniform_buffers_memory.resize(max_frames_in_flight);
    uniform_buffers_mapped.resize(max_frames_in_flight);

    for (size_t i = 0; i < max_frames_in_flight; i++) {
        create_buffer(
            logical_device,
            physical_device,
            uniform_buffer_size, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            uniform_buffers[i], 
            uniform_buffers_memory[i]
        );

        vkMapMemory(logical_device, uniform_buffers_memory[i], 0, uniform_buffer_size, 0, &uniform_buffers_mapped[i]);
    }
}

// MODIFIES: uniform_buffers_mapped
// EFFECTS: Copies the current uniform buffer to the uniform buffers mapped memory
void update_uniform_buffer(uint32_t frame_index, std::vector<void*>& uniform_buffers_mapped) {
    UniformBufferObject uniform_buffer{};
    uniform_buffer.model = glm::mat4(1.0f);
    uniform_buffer.view = glm::mat4(1.0f);
    uniform_buffer.proj = glm::mat4(1.0f);

    memcpy(uniform_buffers_mapped[frame_index], &uniform_buffer, sizeof(uniform_buffer));
}