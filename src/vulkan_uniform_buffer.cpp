#include <iostream>

#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan_uniform_buffer.h>
#include <vulkan_utils.h>

// EFFECTS: Creates a descriptor set layout for the camera information uniform buffer object and returns it
void create_descriptor_set_layout_camera(VkDevice logical_device, VkDescriptorSetLayout& descriptor_set_layout) {
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
        throw std::runtime_error("create_descriptor_set_layout_camera(): Failed to create descriptor set layout!");
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