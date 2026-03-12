#ifndef VULKAN_WINDOW_H
#define VULKAN_WINDOW_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

void setup_window(GLFWwindow*& window, void* user_pointer, GLFWcursorposfun mouse_callback);
void setup_window_surface(VkInstance vk_instance, GLFWwindow* window, VkSurfaceKHR& surface);

#endif