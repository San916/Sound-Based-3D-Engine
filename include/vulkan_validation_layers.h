#ifndef VULKAN_VALIDATION_LAYERS_H
#define VULKAN_VALIDATION_LAYERS_H

#include <vector>

static std::vector<const char*> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

void add_validation_layers(VkInstanceCreateInfo& create_info, std::vector<const char*>& extensions, VkDebugUtilsMessengerCreateInfoEXT& debug_create_info);
void setup_debug_messenger(VkInstance& vk_instance, VkDebugUtilsMessengerEXT& debug_messenger);
void destroy_debug_messenger(VkInstance& vk_instance, VkDebugUtilsMessengerEXT& debug_messenger);

#endif