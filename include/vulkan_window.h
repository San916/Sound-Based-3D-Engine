#ifndef VULKAN_WINDOW_H
#define VULKAN_WINDOW_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

void setup_window(GLFWwindow*& window);
void setup_window_surface(VkInstance vk_instance, GLFWwindow* window, VkSurfaceKHR& surface);

#endif