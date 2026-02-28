#include <iostream>

#include <set>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan_physical_device.h>

#define DESIRED_QUEUE_FAMILY_FLAGS VK_QUEUE_GRAPHICS_BIT

// EFFECTS: Checks if given device supports the required extensions
static bool supports_device_extensions(VkPhysicalDevice physical_device) {
    uint32_t property_count;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &property_count, nullptr);

    std::vector<VkExtensionProperties> extension_properties(property_count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &property_count, extension_properties.data());

    std::set<std::string> required_device_extensions_copy(required_device_extensions.begin(), required_device_extensions.end());
    for (VkExtensionProperties extension : extension_properties) {
        required_device_extensions_copy.erase(extension.extensionName);
    }
    
    return required_device_extensions_copy.empty();
}

// MODIFIES: queue_family_indices
// EFFECTS: Returns true if the device has the right queue families, false otherwise
// There must be a queue family index with the correct flags, and one that can support presenting to the surface given
static bool has_valid_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface, QueueFamilyIndices &queue_family_indices) {
    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    bool queue_family_index_set = false;
    for (uint32_t i = 0; i < queue_families.size(); i++) {
        VkQueueFamilyProperties queue_family = queue_families[i];
        if (queue_family.queueFlags & DESIRED_QUEUE_FAMILY_FLAGS == DESIRED_QUEUE_FAMILY_FLAGS) {
            queue_family_indices.graphics_family_index = i;
            queue_family_index_set = true;
        }

        VkBool32 device_supported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &device_supported);
        if (device_supported == VK_TRUE) {
            queue_family_indices.present_family_index = i;
        }

        if (queue_family_indices.isComplete()) {
            return true;
        }
    }

    return false;
}

// MODIFIES: physical_device, selected_queue_family_index
// EFFECTS: Iterates through devices and sets physical_device to any suitable devices
// Returns selected_queue_family_index with the selected index for the physical device
static void select_physical_device(std::vector<VkPhysicalDevice>& devices, VkPhysicalDevice& physical_device, VkSurfaceKHR surface, QueueFamilyIndices& selected_family_indices) {
    for (VkPhysicalDevice device : devices) {
        QueueFamilyIndices queue_family_indices;
        if (!has_valid_queue_families(device, surface, queue_family_indices)) continue;
        if (!supports_device_extensions(device)) continue;

        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceProperties(device, &device_properties);
        vkGetPhysicalDeviceFeatures(device, &device_features);

        switch (device_properties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                physical_device = physical_device == VK_NULL_HANDLE ? device : physical_device;
                selected_family_indices = queue_family_indices;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                physical_device = device;
                selected_family_indices = queue_family_indices;
                break;
            default:
                break;
        }
    }
}

// MODIFIES: physical_device, queue_family_indices
// EFFECTS: Enumerates on available physical devices (GPUS) and sets physical_device to the suitable GPU
// Returns queue_family_indices of selected physical device
// Throws exception if no devices or no suitable devices
void setup_physical_device(VkInstance& vk_instance, VkPhysicalDevice& physical_device, VkSurfaceKHR surface, QueueFamilyIndices &queue_family_indices) {
    physical_device = VK_NULL_HANDLE;

    uint32_t physical_device_count;
    vkEnumeratePhysicalDevices(vk_instance, &physical_device_count, nullptr);

    if (physical_device_count == 0) {
        throw std::runtime_error("setup_physical_device(): No GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(physical_device_count);
    vkEnumeratePhysicalDevices(vk_instance, &physical_device_count, devices.data());

    select_physical_device(devices, physical_device, surface, queue_family_indices);

    if (physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("setup_physical_device(): Failed to select a suitable GPU!");
    }
}