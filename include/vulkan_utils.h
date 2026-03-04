#ifndef VULKAN_UTILS_H
#define VULKAN_UTILS_H

#include <vulkan/vulkan.h>

void create_buffer(
    VkDevice logical_device,
    VkPhysicalDevice physical_device,
    VkDeviceSize buffer_size,
    VkBufferUsageFlags usage_flags,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& buffer_memory
);

#endif