#include <iostream>

#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan_physical_device.h>

// REQUIRES: List of physical devices, physical device to be set
// MODIFIES: physical_device
// EFFECTS: Iterates through devices and sets physical_device to any suitable devices
static void select_physical_device(std::vector<VkPhysicalDevice>& devices, VkPhysicalDevice& physical_device) {
    for (VkPhysicalDevice device : devices) {
        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceProperties(device, &device_properties);
        vkGetPhysicalDeviceFeatures(device, &device_features);

        switch (device_properties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                physical_device = physical_device == VK_NULL_HANDLE ? device : physical_device;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                physical_device = device;
                break;
            default:
                break;
        }
    }
}

// REQUIRES: vulkan instance and vulkan physical device
// MODIFIES: physical_device
// EFFECTS: Enumerates on available physical devices (GPUS) and sets physical_device to the suitable GPU
// Throws exception if no devices or no suitable devices
void setup_physical_device(VkInstance& vk_instance, VkPhysicalDevice& physical_device) {
    physical_device = VK_NULL_HANDLE;

    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(vk_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("setup_physical_device(): No GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vk_instance, &deviceCount, devices.data());
   
    select_physical_device(devices, physical_device);

    if (physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("setup_physical_device(): Failed to select a suitable GPU!");
    }
}