#include <iostream>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

#include <vulkan_window.h>

static void keypress_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

// MODIFIES: window
// EFFECTS: Creates a GLFW window
//     Sets window to be unresizable
//     Sets the user pointer, such that the mouse callback can access it
//     Sets the mouse callback, which handles mouse movements
//     Hides cursor in the window
//     GLFW_RAW_MOUSE_MOTION removes mouse acceleration
void setup_window(GLFWwindow*& window, void* user_pointer, GLFWcursorposfun mouse_callback) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("setup_window(): Failed to create GLFW window!");
    }

    glfwSetWindowUserPointer(window, user_pointer);
    glfwSetKeyCallback(window, keypress_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
}

// MODIFIES: surface
// EFFECTS: Sets up window surface for the given window
void setup_window_surface(VkInstance vk_instance, GLFWwindow* window, VkSurfaceKHR& surface) {
    if (glfwCreateWindowSurface(vk_instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("setup_window_surface(): Failed to create window surface!");
    }
}

