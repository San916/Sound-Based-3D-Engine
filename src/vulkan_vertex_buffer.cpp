#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vulkan_command_buffer.h>
#include <vulkan_utils.h>
#include <vulkan_vertex_buffer.h>

// MODIFIES: vertex_binding_description
// EFFECTS: Creates vertex input binding description and returns it
void get_vertex_binding_description(VkVertexInputBindingDescription& vertex_binding_description) {
    vertex_binding_description = VkVertexInputBindingDescription{};
    vertex_binding_description.binding = 0;
    vertex_binding_description.stride = sizeof(Vertex);
    vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

// MODIFIES: vertex_attribute_descriptions
// EFFECTS: Creates vertex input attribute descriptions and returns it
void get_vertex_attribute_descriptions(std::vector<VkVertexInputAttributeDescription>& vertex_attribute_descriptions) {
    vertex_attribute_descriptions.resize(1);
    vertex_attribute_descriptions[0].binding = 0;
    vertex_attribute_descriptions[0].location = 0;
    vertex_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_attribute_descriptions[0].offset = offsetof(Vertex, pos);
}

// MODIFIES: vertex_buffer, vertex_buffer_memory
// EFFECTS: Creates vertex buffer handle, and initializes vertex buffer memory using the given vertices
void create_vertex_buffer(
    VkDevice logical_device, 
    VkPhysicalDevice physical_device, 
    VkCommandPool command_pool, 
    VkQueue graphics_queue, 
    const std::vector<Vertex>& vertices, 
    VkBuffer& vertex_buffer, 
    VkDeviceMemory& vertex_buffer_memory
) {
    VkDeviceSize buffer_size = vertices.size() * sizeof(vertices[0]);

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    create_buffer(
        logical_device, 
        physical_device, 
        buffer_size, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer,
        staging_buffer_memory
    );

    void* data;
    vkMapMemory(logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices.data(), (size_t)buffer_size);
    vkUnmapMemory(logical_device, staging_buffer_memory);

    create_buffer(
        logical_device, 
        physical_device, 
        buffer_size, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        vertex_buffer, 
        vertex_buffer_memory
    );

    copy_buffer(logical_device, command_pool, graphics_queue, staging_buffer, vertex_buffer, buffer_size);

    vkDestroyBuffer(logical_device, staging_buffer, nullptr);
    vkFreeMemory(logical_device, staging_buffer_memory, nullptr);
}