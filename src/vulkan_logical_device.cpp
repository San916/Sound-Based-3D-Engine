#include <iostream>

#include <cstdint>
#include <vector>
#include <set>

#include <vulkan/vulkan.h>

#include <vulkan_logical_device.h>
#include <vulkan_physical_device.h>

// MODIFIES: logical_device
// EFFECTS: Sets up logical device
// Uses physical device and queue family indices to setup logical device
// Sets graphics and present queue handles with corresponding queue family indices
void setup_logical_device(
    VkPhysicalDevice physical_device, 
    VkDevice& logical_device, 
    const QueueFamilyIndices& queue_family_indices, 
    VkQueue& graphics_queue, 
    VkQueue& present_queue
) {
    std::vector<VkDeviceQueueCreateInfo> device_queue_create_infos;
    std::set<uint32_t> unique_queue_family_indices = {
        queue_family_indices.graphics_family_index.value(),
        queue_family_indices.present_family_index.value()
    };

    float p_queue_priorities = 1.0f;
    for (uint32_t queue_family_index : unique_queue_family_indices) {
        VkDeviceQueueCreateInfo device_queue_create_info{};
        device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info.pNext = nullptr;
        device_queue_create_info.flags = 0;
        device_queue_create_info.queueFamilyIndex = queue_family_index;
        device_queue_create_info.queueCount = 1;
        device_queue_create_info.pQueuePriorities = &p_queue_priorities;
        device_queue_create_infos.push_back(device_queue_create_info);
    }

    VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address_features{};
    buffer_device_address_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    buffer_device_address_features.pNext = nullptr;
    buffer_device_address_features.bufferDeviceAddress = VK_TRUE;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features{};
    acceleration_structure_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    acceleration_structure_features.pNext = &buffer_device_address_features;
    acceleration_structure_features.accelerationStructure = VK_TRUE;

    VkPhysicalDeviceRayQueryFeaturesKHR ray_query_features{};
    ray_query_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
    ray_query_features.pNext = &acceleration_structure_features;
    ray_query_features.rayQuery = VK_TRUE;

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = &ray_query_features;
    device_create_info.pQueueCreateInfos = device_queue_create_infos.data();
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(device_queue_create_infos.size());
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.ppEnabledExtensionNames = required_device_extensions.data();
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(required_device_extensions.size());

    if (vkCreateDevice(physical_device, &device_create_info, nullptr, &logical_device) != VK_SUCCESS) {
        throw std::runtime_error("setup_logical_device(): Failed to create logical device!");
    }

    vkGetDeviceQueue(logical_device, queue_family_indices.graphics_family_index.value(), 0, &graphics_queue);
    vkGetDeviceQueue(logical_device, queue_family_indices.present_family_index.value(), 0, &present_queue);
}