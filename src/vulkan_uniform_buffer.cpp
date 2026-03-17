#include <iostream>
#include <cstdint>

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
//     Sets the view matrix using the given camera position and rotation
void update_uniform_buffer(
    uint32_t frame_index,
    VkExtent2D swap_chain_extent,
    const std::vector<glm::mat4>& transforms,
    glm::vec3 camera_position,
    glm::vec2 camera_rotation,
    const std::vector<glm::vec4>& sound_waves,
    std::vector<void*>& uniform_buffers_mapped
) {
    UniformBufferObject uniform_buffer{};
    for (size_t i = 0; i < transforms.size() && i < MAX_OBJECTS; i++) {
        uniform_buffer.model[i] = transforms[i];
    }

    glm::vec3 camera_direction;
    camera_direction.x = cos(glm::radians(camera_rotation.x)) * cos(glm::radians(camera_rotation.y));
    camera_direction.y = sin(glm::radians(camera_rotation.y));
    camera_direction.z = sin(glm::radians(camera_rotation.x)) * cos(glm::radians(camera_rotation.y));

    glm::vec3 camera_target = camera_position + camera_direction;
    uniform_buffer.view = glm::lookAt(camera_position, camera_target, glm::vec3(0.0f, 1.0f, 0.0f));
    uniform_buffer.proj = glm::perspective(glm::radians(75.0f), swap_chain_extent.width / (float)swap_chain_extent.height, 0.100f, 1000.0f);
    uniform_buffer.proj[1][1] *= -1;
    uniform_buffer.position = glm::vec4(camera_position, 1.0f);

    for (size_t i = 0; i < MAX_SOUND_WAVES; i++) {
        uniform_buffer.sound_waves[i] = (i < sound_waves.size()) ? sound_waves[i] : glm::vec4(0.0f, 0.0f, 0.0f, -1.0f);
    }

    memcpy(uniform_buffers_mapped[frame_index], &uniform_buffer, sizeof(uniform_buffer));
}