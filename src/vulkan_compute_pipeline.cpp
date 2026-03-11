#include <iostream>

#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan_compute_pipeline.h>
#include <vulkan_utils.h>

// MODIFIES: compute_pipeline_layout, compute_pipeline
// EFFECTS: Creates the compute pipeline
//     Loads compute shader code
//     Uses the given descriptor set layout to create the compute pipeline layout
//     Uses compute pipeline layout to create the compute pipeline
void create_compute_pipeline(
    VkDevice logical_device, 
    VkDescriptorSetLayout descriptor_set_layout, 
    VkPipelineLayout& compute_pipeline_layout, 
    VkPipeline& compute_pipeline
) {
    std::vector<char> comp_shader_code = read_file("./../assets/shaders/shader_comp.spv");
    VkShaderModule comp_shader_module = create_shader_module(logical_device, comp_shader_code);

    VkPipelineShaderStageCreateInfo compute_shader_stage_info{};
    compute_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    compute_shader_stage_info.pNext = nullptr;
    compute_shader_stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    compute_shader_stage_info.module = comp_shader_module;
    compute_shader_stage_info.pName = "main";
    compute_shader_stage_info.pSpecializationInfo = nullptr;

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pNext = nullptr;

    VkDescriptorSetLayout descriptor_set_layouts[] = {descriptor_set_layout};
    pipeline_layout_create_info.setLayoutCount = 1;
    pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts;
    pipeline_layout_create_info.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(logical_device, &pipeline_layout_create_info, nullptr, &compute_pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("create_compute_pipeline(): Failed to create compute pipeline layout!");
    }

    VkComputePipelineCreateInfo compute_pipeline_create_info{};
    compute_pipeline_create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    compute_pipeline_create_info.pNext = nullptr;
    compute_pipeline_create_info.stage = compute_shader_stage_info;
    compute_pipeline_create_info.layout = compute_pipeline_layout;
    compute_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;


    if (vkCreateComputePipelines(logical_device, VK_NULL_HANDLE, 1, &compute_pipeline_create_info, nullptr, &compute_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("create_compute_pipeline(): Failed to create compute pipeline!");
    }

    vkDestroyShaderModule(logical_device, comp_shader_module, nullptr);
}