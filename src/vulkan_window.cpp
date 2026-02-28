#include <iostream>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#include <vulkan_window.h>

void setup_window(GLFWwindow*& window) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("setup_window(): Failed to create GLFW window!");
    }
}

void setup_window_surface(VkInstance vk_instance, GLFWwindow* window, VkSurfaceKHR& surface) {
    if (glfwCreateWindowSurface(vk_instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("setup_window_surface(): Failed to create window surface!");
    }
}

