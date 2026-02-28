#ifndef VULKAN_PHYSICAL_DEVICE_H
#define VULKAN_PHYSICAL_DEVICE_H

#include <vulkan/vulkan.h>

void setup_physical_device(VkInstance& vk_instance, VkPhysicalDevice& physical_device, uint32_t& queue_family_index);

#endif