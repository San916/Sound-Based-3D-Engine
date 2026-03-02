#include <iostream>
#include <fstream>

#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan_graphics_pipeline.h>

// EFFECTS: Read file and return as a char vector
static std::vector<char> read_file(const std::string& file_name) {
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("read_file(): Failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

// EFFECTS: Creates a VkShaderModule using the given shader code
static VkShaderModule create_shader_module(VkDevice logical_device, const std::vector<char>& shader_code) {
    VkShaderModuleCreateInfo shader_create_info{};
    shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_create_info.codeSize = shader_code.size();
    shader_create_info.pCode = reinterpret_cast<const uint32_t*>(shader_code.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(logical_device, &shader_create_info, nullptr, &shader_module) != VK_SUCCESS) {
        throw std::runtime_error("create_shader_module() Failed to create shader module!");
    }

    return shader_module;
}

// EFFECTS: Wraps the vertex and fragment shader modules into create infos for graphics pipeline 
static std::vector<VkPipelineShaderStageCreateInfo> create_shader_stage_info(VkDevice logical_device, VkShaderModule vert_shader_module, VkShaderModule frag_shader_module) {
    VkPipelineShaderStageCreateInfo vert_shader_stage_create_info{};
    vert_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_create_info.pNext = nullptr;
    vert_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_create_info.module = vert_shader_module;
    vert_shader_stage_create_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_create_info{};
    frag_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_create_info.pNext = nullptr;
    frag_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_create_info.module = frag_shader_module;
    frag_shader_stage_create_info.pName = "main";

    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_info = {vert_shader_stage_create_info, frag_shader_stage_create_info};
    return shader_stage_create_info;
}

// EFFECTS: Creates vertex input state create info for graphics pipeline
static VkPipelineVertexInputStateCreateInfo create_vertex_input_state_info() {
    VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
    vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_create_info.pNext = nullptr;
    vertex_input_create_info.vertexBindingDescriptionCount = 0;
    vertex_input_create_info.pVertexBindingDescriptions = nullptr;
    vertex_input_create_info.vertexAttributeDescriptionCount = 0;
    vertex_input_create_info.pVertexAttributeDescriptions = nullptr; 

    return vertex_input_create_info;
}

// EFFECTS: Creates input assembly state create info for graphics pipeline
static VkPipelineInputAssemblyStateCreateInfo create_input_assembly_state_info() {
    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info{};
    input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.pNext = nullptr;
    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

    return input_assembly_create_info;
}

// EFFECTS: Creates viewport state create info for graphics pipeline
// Currently uses a static viewport
static VkPipelineViewportStateCreateInfo create_viewport_state_info(VkExtent2D swap_chain_extent) {
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swap_chain_extent.width;
    viewport.height = (float) swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;

    VkPipelineViewportStateCreateInfo viewport_state_create_info{};
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissor;

    return viewport_state_create_info;
}

// EFFECTS: Creates rasterization state create info for graphics pipeline
static VkPipelineRasterizationStateCreateInfo create_rasterization_state_info() {
    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
    rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_create_info.pNext = nullptr;
    rasterization_state_create_info.depthClampEnable = VK_FALSE;
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state_create_info.lineWidth = 1.0f;
    rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterization_state_create_info.depthBiasEnable = VK_FALSE;

    return rasterization_state_create_info;
}

// EFFECTS: Creates multisample state create info for graphics pipeline
static VkPipelineMultisampleStateCreateInfo create_multisample_state_info() {
    VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
    multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_create_info.pNext = nullptr;
    multisample_state_create_info.sampleShadingEnable = VK_FALSE;
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    return multisample_state_create_info;
}

// EFFECTS: Creates color blend state create info for graphics pipeline
static VkPipelineColorBlendStateCreateInfo create_color_blend_state_info() {
    VkPipelineColorBlendAttachmentState color_blend_attachment_state_create_info{};
    color_blend_attachment_state_create_info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment_state_create_info.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
    color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state_create_info;
    color_blend_state_create_info.blendConstants[0] = 0.0f;
    color_blend_state_create_info.blendConstants[1] = 0.0f;
    color_blend_state_create_info.blendConstants[2] = 0.0f;
    color_blend_state_create_info.blendConstants[3] = 0.0f;

    return color_blend_state_create_info;
}


// MODIFIES: pipeline_layout
// EFFECTS: Creates graphics pipeline into pipeline_layout
void create_graphics_pipeline(VkDevice logical_device, VkExtent2D swap_chain_extent, VkPipelineLayout& pipeline_layout) {
    std::vector<char> vert_shader_code = read_file("./../assets/shaders/shader_vert.spv");
    std::vector<char> frag_shader_code = read_file("./../assets/shaders/shader_frag.spv");

    VkShaderModule vert_shader_module = create_shader_module(logical_device, vert_shader_code);
    VkShaderModule frag_shader_module = create_shader_module(logical_device, frag_shader_code);

    const std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_info = create_shader_stage_info(logical_device, vert_shader_module, frag_shader_module);
    const VkPipelineVertexInputStateCreateInfo vertex_input_create_info = create_vertex_input_state_info();
    const VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = create_input_assembly_state_info();
    const VkPipelineViewportStateCreateInfo viewport_state_create_info = create_viewport_state_info(swap_chain_extent);
    const VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = create_rasterization_state_info();
    const VkPipelineMultisampleStateCreateInfo multisample_state_create_info = create_multisample_state_info();
    const VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = create_color_blend_state_info();

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pNext = nullptr;
    pipeline_layout_create_info.setLayoutCount = 0;
    pipeline_layout_create_info.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(logical_device, &pipeline_layout_create_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("create_graphics_pipeline(): Failed to create graphics pipeline layout!");
    }

    vkDestroyShaderModule(logical_device, vert_shader_module, nullptr);
    vkDestroyShaderModule(logical_device, frag_shader_module, nullptr);
}