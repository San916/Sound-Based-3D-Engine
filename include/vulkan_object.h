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

struct ObjectProperties {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

    int visible = 1;
    int emitting = 0;
    float emit_cooldown = 0.0f;
    float emit_interval = 0.5f;

    int physics_enabled = 1;
    float mass = 1.0f;

    glm::mat4 get_model_matrix() const;
};

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

    void init_object(VkPhysicalDevice physical_device, VkCommandPool command_pool, VkQueue graphics_queue);
public:
    ObjectProperties properties;

    VulkanObject(
        const std::string& file_name,
        VkDevice logical_device,
        VkPhysicalDevice physical_device,
        VkCommandPool command_pool,
        VkQueue graphics_queue
    );
    ~VulkanObject();

    VulkanObject(const VulkanObject&) = delete;
    VulkanObject& operator=(const VulkanObject&) = delete;

    VkAccelerationStructureKHR get_blas() const { return blas; }
    VkBuffer get_vertex_buffer() const { return vertex_buffer; }
    VkBuffer get_index_buffer() const { return index_buffer; }
    const std::vector<uint32_t>& get_indices() const { return indices; }
};

#endif
