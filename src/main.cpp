#include <iostream>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vulkan_handle.h>

int main() {
    glfwInit();

    try {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
        if (window == NULL) {
            glfwTerminate();
            return EXIT_FAILURE;
        }

        VulkanHandle* vulkan_handle = new VulkanHandle();

        while(!glfwWindowShouldClose(window)) {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
            glfwPollEvents();
        }

        delete vulkan_handle;

        glfwDestroyWindow(window);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
    }

    glfwTerminate();

    return 0;
}