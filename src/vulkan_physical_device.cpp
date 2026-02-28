#include <iostream>

#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan_physical_device.h>

#define DESIRED_QUEUE_FAMILY_FLAGS VK_QUEUE_GRAPHICS_BIT

// REQUIRES: Physical device to check, queue_family_index to be set and returned
// EFFECTS: Returns true of the device has the right queue families, false otherwise
// Sets queue_family_index
static bool has_valid_queue_families(VkPhysicalDevice &physical_device, uint32_t& queue_family_index) {
    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    for (uint32_t i = 0; i < queue_families.size(); i++) {
        VkQueueFamilyProperties queue_family = queue_families[i];
        if (queue_family.queueFlags & DESIRED_QUEUE_FAMILY_FLAGS == DESIRED_QUEUE_FAMILY_FLAGS) {
            queue_family_index = i;
            return true;
        }
    }

    return false;
}

// REQUIRES: List of physical devices, physical device to be set, queue_family_index to select
// MODIFIES: physical_device, selected_queue_family_index
// EFFECTS: Iterates through devices and sets physical_device to any suitable devices
// Sets selected_queue_family_index with the one for the selected physical device
static void select_physical_device(std::vector<VkPhysicalDevice>& devices, VkPhysicalDevice& physical_device, uint32_t& selected_queue_family_index) {
    for (VkPhysicalDevice device : devices) {
        uint32_t queue_family_index;
        if (!has_valid_queue_families(device, queue_family_index)) continue;

        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceProperties(device, &device_properties);
        vkGetPhysicalDeviceFeatures(device, &device_features);

        switch (device_properties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                physical_device = physical_device == VK_NULL_HANDLE ? device : physical_device;
                selected_queue_family_index = queue_family_index;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                physical_device = device;
                selected_queue_family_index = queue_family_index;
                break;
            default:
                break;
        }
    }
}

// REQUIRES: vulkan instance and vulkan physical device, queue family index to be returned
// MODIFIES: physical_device, queue_family_index
// EFFECTS: Enumerates on available physical devices (GPUS) and sets physical_device to the suitable GPU
// queue_family_index used to create logical device
// Throws exception if no devices or no suitable devices
void setup_physical_device(VkInstance& vk_instance, VkPhysicalDevice& physical_device, uint32_t& queue_family_index) {
    physical_device = VK_NULL_HANDLE;

    uint32_t physical_device_count;
    vkEnumeratePhysicalDevices(vk_instance, &physical_device_count, nullptr);

    if (physical_device_count == 0) {
        throw std::runtime_error("setup_physical_device(): No GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(physical_device_count);
    vkEnumeratePhysicalDevices(vk_instance, &physical_device_count, devices.data());

    select_physical_device(devices, physical_device, queue_family_index);

    if (physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("setup_physical_device(): Failed to select a suitable GPU!");
    }
}