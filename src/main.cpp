#include <iostream>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <vulkan_handle.h>

int main() {
    try {
        VulkanHandle* vulkan_handle = new VulkanHandle();

        vulkan_handle->run();

        delete vulkan_handle;
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
    }

    return 0;
}