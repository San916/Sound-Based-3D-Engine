#include <iostream>

#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan_uniform_buffer.h>
#include <vulkan_utils.h>

// EFFECTS: Creates a descriptor set layout and returns it
// Currently specifies vertex and fragment shader usage
void create_descriptor_set_layout(VkDevice logical_device, VkDescriptorSetLayout& descriptor_set_layout) {
    VkDescriptorSetLayoutBinding descriptor_set_layout_binding{};
    descriptor_set_layout_binding.binding = 0;
    descriptor_set_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_set_layout_binding.descriptorCount = 1;
    descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptor_set_layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.pNext = nullptr;
    descriptor_set_layout_create_info.bindingCount = 1;
    descriptor_set_layout_create_info.pBindings = &descriptor_set_layout_binding;

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

void create_descriptor_sets(
    VkDevice logical_device, 
    size_t max_frames_in_flight, 
    VkDescriptorPool descriptor_pool,
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
}

void create_uniform_buffers(
    VkDevice logical_device,
    VkPhysicalDevice physical_device,
    size_t max_frames_in_flight,
    std::vector<VkBuffer>& uniform_buffers,
    std::vector<VkDeviceMemory>& uniform_buffers_memory,
    std::vector<void*>& uniform_buffers_mapped
) {
    VkDeviceSize uniform_buffer_size = sizeof(CameraUBO);

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