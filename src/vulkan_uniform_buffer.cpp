#include <iostream>

#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan_uniform_buffer.h>
#include <vulkan_utils.h>

// EFFECTS: Creates a descriptor set layout and returns it
// Currently specifies vertex and fragment shader usage
void create_descriptor_set_layout(VkDevice logical_device, VkDescriptorSetLayout& descriptor_set_layout) {
    std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;

    VkDescriptorSetLayoutBinding uniform_buffer_binding{};
    uniform_buffer_binding.binding = 0;
    uniform_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_buffer_binding.descriptorCount = 1;
    uniform_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uniform_buffer_binding.pImmutableSamplers = nullptr;

    descriptor_set_layout_bindings.push_back(uniform_buffer_binding);

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.pNext = nullptr;
    descriptor_set_layout_create_info.bindingCount = 1;
    descriptor_set_layout_create_info.pBindings = descriptor_set_layout_bindings.data();

    if (vkCreateDescriptorSetLayout(logical_device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
        throw std::runtime_error("create_descriptor_set_layout(): Failed to create descriptor set layout!");
    }
}

// MODIFIES: descriptor_pool
// EFFECTS: Creates descriptor pool with entries equal to the number of inflight frames
void create_descriptor_pool(VkDevice logical_device, size_t max_frames_in_flight, VkDescriptorPool& descriptor_pool) {
    VkDescriptorPoolSize descriptor_pool_size{};
    descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_pool_size.descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

    VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.pNext = nullptr;
    descriptor_pool_create_info.maxSets = static_cast<uint32_t>(max_frames_in_flight);
    descriptor_pool_create_info.poolSizeCount = 1;
    descriptor_pool_create_info.pPoolSizes = &descriptor_pool_size;

    if (vkCreateDescriptorPool(logical_device, &descriptor_pool_create_info, nullptr, &descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("create_descriptor_pool(): Failed to create descriptor pool!");
    }
}

// REQUIRES: uniform_buffers.size(), descriptor_set_layout.size() >= max_frames_in_flight
// MODIFIES: descriptor_sets
// EFFECTS: Allocates descriptor sets and binds each descriptor set to their respective uniform buffer
void create_descriptor_sets(
    VkDevice logical_device, 
    size_t max_frames_in_flight, 
    VkDescriptorPool descriptor_pool,
    const std::vector<VkBuffer>& uniform_buffers,
    const VkDescriptorSetLayout& descriptor_set_layout,
    std::vector<VkDescriptorSet>& descriptor_sets
) {
    descriptor_sets.resize(max_frames_in_flight);

    std::vector<VkDescriptorSetLayout> descriptor_set_layouts(max_frames_in_flight, descriptor_set_layout);
    VkDescriptorSetAllocateInfo descriptor_set_alloc_info{};
    descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_alloc_info.pNext = nullptr;
    descriptor_set_alloc_info.descriptorPool = descriptor_pool;
    descriptor_set_alloc_info.descriptorSetCount = static_cast<uint32_t>(max_frames_in_flight);
    descriptor_set_alloc_info.pSetLayouts = descriptor_set_layouts.data();

    if (vkAllocateDescriptorSets(logical_device, &descriptor_set_alloc_info, descriptor_sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("create_descriptor_sets(): Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < max_frames_in_flight; i++) {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet write_descriptor{};
        write_descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor.pNext = nullptr;
        write_descriptor.dstSet = descriptor_sets[i];
        write_descriptor.dstBinding = 0;
        write_descriptor.dstArrayElement = 0;
        write_descriptor.descriptorCount = 1;
        write_descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptor.pImageInfo = nullptr;
        write_descriptor.pBufferInfo = &buffer_info;
        write_descriptor.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(logical_device, 1, &write_descriptor, 0, nullptr);
    }
}

// MODIFIES: uniform_buffers, uniform_buffers_memory, uniform_buffers_mapped
// EFFECTS: Creates uniform buffer handles, uniform buffer memory, and uniform buffer maps
void create_uniform_buffers(
    VkDevice logical_device,
    VkPhysicalDevice physical_device,
    size_t max_frames_in_flight,
    std::vector<VkBuffer>& uniform_buffers,
    std::vector<VkDeviceMemory>& uniform_buffers_memory,
    std::vector<void*>& uniform_buffers_mapped
) {
    VkDeviceSize uniform_buffer_size = sizeof(UniformBufferObject);

    uniform_buffers.resize(max_frames_in_flight);
    uniform_buffers_memory.resize(max_frames_in_flight);
    uniform_buffers_mapped.resize(max_frames_in_flight);

    for (size_t i = 0; i < max_frames_in_flight; i++) {
        create_buffer(
            logical_device,
            physical_device,
            uniform_buffer_size, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            uniform_buffers[i], 
            uniform_buffers_memory[i]
        );

        vkMapMemory(logical_device, uniform_buffers_memory[i], 0, uniform_buffer_size, 0, &uniform_buffers_mapped[i]);
    }
}

// MODIFIES: uniform_buffers_mapped
// EFFECTS: Copies the current uniform buffer to the uniform buffers mapped memory
void update_uniform_buffer(uint32_t frame_index, std::vector<void*>& uniform_buffers_mapped) {
    UniformBufferObject uniform_buffer{};
    uniform_buffer.model = glm::mat4(1.0f);
    uniform_buffer.view = glm::mat4(1.0f);
    uniform_buffer.proj = glm::mat4(1.0f);

    memcpy(uniform_buffers_mapped[frame_index], &uniform_buffer, sizeof(uniform_buffer));
}