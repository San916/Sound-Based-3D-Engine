#ifndef VULKAN_UTILS_H
#define VULKAN_UTILS_H

#include <cstdint>

#include <string>

#include <vulkan/vulkan.h>

uint32_t get_memory_type(VkPhysicalDevice physical_device, uint32_t compatible_types, VkMemoryPropertyFlags properties);
void create_buffer(
    VkDevice logical_device,
    VkPhysicalDevice physical_device,
    VkDeviceSize buffer_size,
    VkBufferUsageFlags usage_flags,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& buffer_memory
);
VkShaderModule create_shader_module(VkDevice logical_device, const std::vector<char>& shader_code);
std::vector<char> read_file(const std::string& file_name);

#endif