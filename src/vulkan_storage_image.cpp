#include <iostream>
#include <cstdint>

#include <vulkan/vulkan.h>

#include <vulkan_command_buffer.h>
#include <vulkan_physical_device.h>
#include <vulkan_storage_image.h>
#include <vulkan_utils.h>

// MODIFIES: storage_image, storage_image_memory
// EFFECTS: 
//     Creates a storage image in device memory then binds a VkImage to that memory
static void create_storage_image_memory(
    VkDevice logical_device, VkPhysicalDevice physical_device, 
    VkExtent2D swap_chain_extent, VkFormat image_format, VkImageUsageFlags usage_flags,
    VkImage& storage_image, VkDeviceMemory& storage_image_memory
) {
    VkExtent3D extent_3d{};
    extent_3d.width = swap_chain_extent.width;
    extent_3d.height = swap_chain_extent.height;
    extent_3d.depth = 1;

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext = nullptr;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = image_format;
    image_info.extent = extent_3d;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = usage_flags;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (vkCreateImage(logical_device, &image_info, nullptr, &storage_image) != VK_SUCCESS) {
        throw std::runtime_error("create_storage_image_memory(): Failed to create image!");
    }

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(logical_device, storage_image, &memory_requirements);

    VkMemoryAllocateInfo memory_alloc_info{};
    memory_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_alloc_info.pNext = nullptr;
    memory_alloc_info.allocationSize = memory_requirements.size;
    memory_alloc_info.memoryTypeIndex = get_memory_type(
        physical_device, 
        memory_requirements.memoryTypeBits, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    if (vkAllocateMemory(logical_device, &memory_alloc_info, nullptr, &storage_image_memory) != VK_SUCCESS) {
        throw std::runtime_error("create_storage_image_memory(): Failed to allocate image memory!");
    }
    vkBindImageMemory(logical_device, storage_image, storage_image_memory, 0);
}

// MODIFIES: storage_image_view
// EFFECTS: Creates the image view for the given storage image
static void create_storage_image_view(
    VkDevice logical_device, VkFormat image_format, 
    VkImage& storage_image, VkImageView& storage_image_view
) {
    VkImageViewCreateInfo image_view_info{};
    image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_info.pNext = nullptr;
    image_view_info.image = storage_image;
    image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_info.format = image_format;
    image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_info.subresourceRange.baseMipLevel = 0;
    image_view_info.subresourceRange.levelCount = 1;
    image_view_info.subresourceRange.baseArrayLayer = 0;
    image_view_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(logical_device, &image_view_info, nullptr, &storage_image_view) != VK_SUCCESS) {
        throw std::runtime_error("create_storage_image_view(): Failed to create image view!");
    }
}

// MODIFIES: storage_image, storage_image_memory, storage_image_view
// EFFECTS:
//     Creates storage image memory and binds storage_image to it
//     Creates storage image memory view using storage_image
//     Transitions the image memory layout to VK_IMAGE_LAYOUT_GENERAL, allowing device access
//     Transitions access mask to VK_ACCESS_SHADER_WRITE_BIT, allowing shader write access
void create_storage_image(
    VkDevice logical_device, VkPhysicalDevice physical_device, 
    VkCommandPool command_pool, VkQueue graphics_queue, 
    VkExtent2D swap_chain_extent, 
    VkFormat storage_image_format, VkImageUsageFlags usage_flags,
    VkImage& storage_image, VkDeviceMemory& storage_image_memory, 
    VkImageView& storage_image_view
) {
    create_storage_image_memory(
        logical_device, physical_device, 
        swap_chain_extent, storage_image_format, usage_flags,
        storage_image, storage_image_memory
    );

    create_storage_image_view(
        logical_device, storage_image_format, 
        storage_image, storage_image_view
    );

    VkCommandBuffer command_buffer;
    begin_single_time_command(logical_device, command_pool, command_buffer);

    VkImageMemoryBarrier image_memory_barrier{};
    image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_memory_barrier.pNext = nullptr;
    image_memory_barrier.srcAccessMask = 0;
    image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.image = storage_image;
    image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_barrier.subresourceRange.baseMipLevel = 0;
    image_memory_barrier.subresourceRange.levelCount = 1;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0;
    image_memory_barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        command_buffer, 
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 
        0, 0, nullptr, 0, nullptr, 1, 
        &image_memory_barrier
    );

    finish_single_time_command(logical_device, graphics_queue, command_pool, command_buffer);
}

// MODIFIES: storage_image, storage_image_memory, storage_image_view
void cleanup_storage_image(
    VkDevice logical_device, 
    VkImage& storage_image,  
    VkDeviceMemory& storage_image_memory, 
    VkImageView& storage_image_view
) {
    vkDestroyImageView(logical_device, storage_image_view, nullptr);
    vkDestroyImage(logical_device, storage_image, nullptr);
    vkFreeMemory(logical_device, storage_image_memory, nullptr);
}