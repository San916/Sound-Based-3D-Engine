#include <iostream>

#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan_logical_device.h>
#include <vulkan_physical_device.h>

// MODIFIES: logical_device
// EFFECTS: Sets up logical device
void setup_logical_device(VkPhysicalDevice& physical_device, VkDevice& logical_device, uint32_t queue_family_index) {
    VkDeviceQueueCreateInfo device_queue_create_info{};
    device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    device_queue_create_info.pNext = nullptr;
    device_queue_create_info.queueFamilyIndex = queue_family_index;
    device_queue_create_info.queueCount = 1;
    float p_queue_priorities = 1.0f;
    device_queue_create_info.pQueuePriorities = &p_queue_priorities;

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = &device_queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pEnabledFeatures = nullptr;
    device_create_info.ppEnabledExtensionNames = required_device_extensions.data();
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(required_device_extensions.size());

    if (vkCreateDevice(physical_device, &device_create_info, nullptr, &logical_device) != VK_SUCCESS) {
        throw std::runtime_error("setup_logical_device(): Failed to create logical device!");
    }
}
