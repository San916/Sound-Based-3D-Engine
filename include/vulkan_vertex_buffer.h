#ifndef VULKAN_VERTEX_BUFFER_H
#define VULKAN_VERTEX_BUFFER_H

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

typedef struct Vertex {
    glm::vec3 pos;
} Vertex;

void get_vertex_binding_description(VkVertexInputBindingDescription& vertex_binding_description);
void get_vertex_attribute_descriptions(std::vector<VkVertexInputAttributeDescription>& vertex_attribute_descriptions);
void create_vertex_buffer(
    VkDevice logical_device, 
    VkPhysicalDevice physical_device, 
    VkCommandPool command_pool, 
    VkQueue graphics_queue, 
    const std::vector<Vertex>& vertices, 
    VkBuffer& vertex_buffer, 
    VkDeviceMemory& vertex_buffer_memory
);

#endif