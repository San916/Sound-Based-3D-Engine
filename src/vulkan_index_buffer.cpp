#include <cstdint>

#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan_command_buffer.h>
#include <vulkan_index_buffer.h>
#include <vulkan_utils.h>

// MODIFIES: index_buffer, index_buffer_memory
// EFFECTS: Creates index buffer handle, and initializes index buffer memory using the given indices
void create_index_buffer(
    VkDevice logical_device, 
    VkPhysicalDevice physical_device,
    VkCommandPool command_pool, 
    VkQueue graphics_queue, 
    const std::vector<uint32_t>& indices, 
    VkBuffer& index_buffer, 
    VkDeviceMemory& index_buffer_memory
) {
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    VkDeviceSize buffer_size = indices.size() * sizeof(indices[0]);

    create_buffer(
        logical_device, 
        physical_device, 
        buffer_size, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer,
        staging_buffer_memory
    );

    void* data;
    vkMapMemory(logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, indices.data(), (size_t)buffer_size);
    vkUnmapMemory(logical_device, staging_buffer_memory);

    create_buffer(
        logical_device, 
        physical_device, 
        buffer_size, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | 
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        index_buffer, 
        index_buffer_memory
    );

    copy_buffer(logical_device, command_pool, graphics_queue, staging_buffer, index_buffer, buffer_size);

    vkDestroyBuffer(logical_device, staging_buffer, nullptr);
    vkFreeMemory(logical_device, staging_buffer_memory, nullptr);
}