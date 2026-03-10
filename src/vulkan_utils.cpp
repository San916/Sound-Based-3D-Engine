#include <iostream>
#include <cstdint>

#include <vulkan/vulkan.h>

#include <vulkan_utils.h>

// EFFECTS: Gets memory properties of given physical_device and returns an index, if exists, where it's both compatible and has the right properties
uint32_t get_memory_type(VkPhysicalDevice physical_device, uint32_t compatible_types, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((compatible_types & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("get_memory_type(): Failed to find suitable memory type!");
}

// MODIFIES: buffer, buffer_memory
// EFFECTS: Creates a buffer, allocates buffer_memory and binds buffer to that memory
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

    VkMemoryAllocateFlagsInfo memory_alloc_flags{};

    VkMemoryAllocateInfo memory_alloc_info{};
    memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    if ((usage_flags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {  
        memory_alloc_flags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        memory_alloc_flags.pNext = nullptr;
        memory_alloc_flags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        memory_alloc_info.pNext = &memory_alloc_flags;
    } else {
        memory_alloc_info.pNext = nullptr;
    }

    memory_alloc_info.allocationSize = memory_requirements.size;
    memory_alloc_info.memoryTypeIndex = get_memory_type(physical_device, memory_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(logical_device, &memory_alloc_info, nullptr, &buffer_memory) != VK_SUCCESS) {
        throw std::runtime_error("create_buffer(): Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(logical_device, buffer, buffer_memory, 0);
}