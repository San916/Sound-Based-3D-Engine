#include <iostream>

#include <vulkan/vulkan.h>

#include <vulkan_utils.h>

static uint32_t get_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("get_memory_type(): Failed to find suitable memory type!");
}

void create_buffer(
    VkDevice logical_device,
    VkPhysicalDevice physical_device,
    VkDeviceSize buffer_size,
    VkBufferUsageFlags usage_flags,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& buffer_memory
) {
    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.pNext = nullptr;
    buffer_create_info.size = buffer_size;
    buffer_create_info.usage = usage_flags;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(logical_device, &buffer_create_info, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("create_buffer(): Failed to create buffer!");
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(logical_device, buffer, &memory_requirements);

    VkMemoryAllocateInfo memory_alloc_info{};
    memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_alloc_info.pNext = nullptr;
    memory_alloc_info.allocationSize = memory_requirements.size;
    memory_alloc_info.memoryTypeIndex = get_memory_type(physical_device, memory_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(logical_device, &memory_alloc_info, nullptr, &buffer_memory) != VK_SUCCESS) {
        throw std::runtime_error("create_buffer(): Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(logical_device, buffer, buffer_memory, 0);
}