#ifndef VULKAN_PHYSICAL_DEVICE_H
#define VULKAN_PHYSICAL_DEVICE_H

#include <optional>

#include <vector>

#include <vulkan/vulkan.h>

typedef struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family_index;
    std::optional<uint32_t> present_family_index;

    bool isComplete() {
        return graphics_family_index.has_value() && present_family_index.has_value();
    }
} QueueFamilyIndices;

extern const std::vector<const char*> required_device_extensions;

void setup_physical_device(VkInstance& vk_instance, VkPhysicalDevice& physical_device, VkSurfaceKHR surface, QueueFamilyIndices &queue_family_indices);

#endif