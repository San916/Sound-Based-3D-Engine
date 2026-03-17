#ifndef VULKAN_ACCELERATION_STRUCTURE_H
#define VULKAN_ACCELERATION_STRUCTURE_H

#include <cstdint>

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vulkan_vertex_buffer.h>

void create_bottom_level_acceleration_structure(
    VkDevice logical_device, VkPhysicalDevice physical_device, 
    VkCommandPool command_pool, VkQueue graphics_queue, 
    VkBuffer vertex_buffer, VkBuffer index_buffer, 
    const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, 
    VkBuffer& blas_buffer, VkDeviceMemory& blas_buffer_memory, VkAccelerationStructureKHR& blas
);
void create_top_level_acceleration_structure(
    VkDevice logical_device, VkPhysicalDevice physical_device,
    VkCommandPool command_pool, VkQueue graphics_queue,
    const std::vector<VkAccelerationStructureKHR>& blases,
    const std::vector<glm::mat4>& transforms,
    VkBuffer& tlas_buffer, VkDeviceMemory& tlas_buffer_memory, VkAccelerationStructureKHR& tlas
);
void cleanup_acceleration_structure(VkDevice logical_device, VkBuffer& as_buffer, VkDeviceMemory& as_buffer_memory, VkAccelerationStructureKHR& acceleration_structure);

#endif