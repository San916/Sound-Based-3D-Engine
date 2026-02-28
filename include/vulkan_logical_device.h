#ifndef VULKAN_LOGICAL_DEVICE_H
#define VULKAN_LOGICAL_DEVICE_H

void setup_logical_device(VkPhysicalDevice& physical_device, VkDevice& logical_device, uint32_t queue_family_index);

#endif