#ifndef VULKAN_LOGICAL_DEVICE_H
#define VULKAN_LOGICAL_DEVICE_H

#include <vulkan/vulkan.h>

typedef struct QueueFamilyIndices QueueFamilyIndices;

void setup_logical_device(VkPhysicalDevice physical_device, VkDevice& logical_device, QueueFamilyIndices queue_family_indices, VkQueue& graphics_queue, VkQueue& present_queue);

#endif