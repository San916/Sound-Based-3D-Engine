#include <iostream>

#include <vector>
#include <string.h>

#include <vulkan/vulkan.h>

#include <vulkan_validation_layers.h>

// Debug callback used in add_validation_layers()
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, 
    VkDebugUtilsMessageTypeFlagsEXT message_types, 
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void* p_user_data) {
    std::cerr << "validation layer: " << p_callback_data->pMessage << std::endl;

    return VK_FALSE;
}

// MODIFIES: debug_messenger_create_info
// EFFECTS: sets up debug create info
static void setup_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT& debug_messenger_create_info) {
    debug_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_messenger_create_info.pfnUserCallback = debug_callback;
}

// REQUIRES: List of validation layers
// EFFECTS: Checks if validation layers are supported/exist, if any are not, returns false
static bool check_validation_layer_support() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> avaliable_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, avaliable_layers.data());

    for (const char* layer_name : validation_layers) {
        bool layer_exists = false;

        for (const VkLayerProperties cur_layer : avaliable_layers) {
            if (strcmp(layer_name, cur_layer.layerName) == 0) {
                layer_exists = true;
                break;
            }
        }

        if (!layer_exists) {
            return false;
        }
    }

    return true;
}


// Does what VkCreateDebugUtilsMessengerEXT() would do
// Use instead of VkCreateDebugUtilsMessengerEXT
static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// Does what VkDestroyDebugUtilsMessengerEXT() would do
// Use instead of VkDestroyDebugUtilsMessengerEXT
static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


// REQUIRES: vulkan instance create info and glfw extensions
// MODIFIES: create_info, extensions
// EFFECTS: Adds validation layers to create_info and extensions
void add_validation_layers(
    VkInstanceCreateInfo& create_info, 
    std::vector<const char*>& extensions, 
    VkDebugUtilsMessengerCreateInfoEXT& debug_create_info) {
    if (!check_validation_layer_support()) {
        throw std::runtime_error("add_validation_layers(): Validation layers unsupported!");
    }

    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.data();

    setup_debug_messenger_create_info(debug_create_info);
    create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;
}


// REQUIRES: vulkan instance and debug messenger
// MODIFIES: vk_instance, debug_messenger
// EFFECTS: sets up debug messenger for the provided instance
void setup_debug_messenger(VkInstance& vk_instance, VkDebugUtilsMessengerEXT& debug_messenger) {
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
    setup_debug_messenger_create_info(debug_create_info);

    if (CreateDebugUtilsMessengerEXT(vk_instance, &debug_create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void destroy_debug_messenger(VkInstance& vk_instance, VkDebugUtilsMessengerEXT& debug_messenger) {
    if (debug_messenger != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(vk_instance, debug_messenger, nullptr);
        debug_messenger = VK_NULL_HANDLE;
    }
}
