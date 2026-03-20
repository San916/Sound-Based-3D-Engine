#ifndef VULKAN_STORAGE_IMAGE_H
#define VULKAN_STORAGE_IMAGE_H

#include <vulkan/vulkan.h>

void create_storage_image(
    VkDevice logical_device, VkPhysicalDevice physical_device, 
    VkCommandPool command_pool, VkQueue graphics_queue, 
    VkExtent2D swap_chain_extent, 
    VkFormat storage_image_format, VkImageUsageFlags usage_flags,
    VkImage& storage_image, VkDeviceMemory& storage_image_memory, 
    VkImageView& storage_image_view
);
void cleanup_storage_image(
    VkDevice logical_device, 
    VkImage& storage_image,  
    VkDeviceMemory& storage_image_memory, 
    VkImageView& storage_image_view
);

#endif