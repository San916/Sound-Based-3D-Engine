#ifndef VULKAN_INDEX_BUFFER_H
#define VULKAN_INDEX_BUFFER_H

#include <cstdint>

#include <vector>

#include <vulkan/vulkan.h>

void create_index_buffer(
    VkDevice logical_device, 
    VkPhysicalDevice physical_device,
    VkCommandPool command_pool, 
    VkQueue graphics_queue, 
    const std::vector<uint32_t>& indices, 
    VkBuffer& index_buffer, 
    VkDeviceMemory& index_buffer_memory
);

#endif