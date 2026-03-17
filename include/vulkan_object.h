#ifndef VULKAN_OBJECT_H
#define VULKAN_OBJECT_H

#include <iostream>
#include <cstdint>

#include <string>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vulkan_index_buffer.h>
#include <vulkan_vertex_buffer.h>

class VulkanObject {
private:
    std::string file_name;
    VkDevice logical_device = VK_NULL_HANDLE;

    std::vector<Vertex> vertices;
    VkBuffer vertex_buffer = VK_NULL_HANDLE;
    VkDeviceMemory vertex_buffer_memory = VK_NULL_HANDLE;

    std::vector<uint32_t> indices;
    VkBuffer index_buffer = VK_NULL_HANDLE;
    VkDeviceMemory index_buffer_memory = VK_NULL_HANDLE;

    VkBuffer blas_buffer = VK_NULL_HANDLE;
    VkDeviceMemory blas_buffer_memory = VK_NULL_HANDLE;
    VkAccelerationStructureKHR blas = VK_NULL_HANDLE;
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    VulkanObject(const std::string& file_name);
    ~VulkanObject();

    void init_object(VkDevice logical_device, VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphics_queue);

    glm::mat4 get_model_matrix() const;
    VkAccelerationStructureKHR get_blas() const { return blas; }
    VkBuffer get_vertex_buffer() const { return vertex_buffer; }
    VkBuffer get_index_buffer() const { return index_buffer; }
    const std::vector<uint32_t>& get_indices() const { return indices; }
};

#endif
