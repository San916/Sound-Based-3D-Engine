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
void setup_logical_device(VkPhysicalDevice physical_device, VkDevice& logical_device, QueueFamilyIndices queue_family_indices, VkQueue& graphics_queue, VkQueue& present_queue) {
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


    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
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
