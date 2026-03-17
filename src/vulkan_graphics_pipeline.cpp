#include <iostream>

#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan_graphics_pipeline.h>
#include <vulkan_utils.h>
#include <vulkan_vertex_buffer.h>

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
static VkPipelineVertexInputStateCreateInfo create_vertex_input_state_info(VkVertexInputBindingDescription& vertex_binding_description, std::vector<VkVertexInputAttributeDescription>& vertex_attribute_descriptions) {
    get_vertex_binding_description(vertex_binding_description);
    get_vertex_attribute_descriptions(vertex_attribute_descriptions);

    VkPipelineVertexInputStateCreateInfo vertex_input_create_info{};
    vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_create_info.pNext = nullptr;
    vertex_input_create_info.vertexBindingDescriptionCount = 1;
    vertex_input_create_info.pVertexBindingDescriptions = &vertex_binding_description;
    vertex_input_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_descriptions.size());
    vertex_input_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data(); 

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

// MODIFIES: viewport, scissor
// EFFECTS: Creates viewport state create info for graphics pipeline
// Currently uses a static viewport
static VkPipelineViewportStateCreateInfo create_viewport_state_info(VkExtent2D swap_chain_extent, VkViewport& viewport, VkRect2D& scissor) {
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swap_chain_extent.width;
    viewport.height = (float) swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

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

// MODIFIES: color_blend_attachment
// EFFECTS: Creates color blend state create info for graphics pipeline
static VkPipelineColorBlendStateCreateInfo create_color_blend_state_info(VkPipelineColorBlendAttachmentState& color_blend_attachment) {
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
    color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment;
    color_blend_state_create_info.blendConstants[0] = 0.0f;
    color_blend_state_create_info.blendConstants[1] = 0.0f;
    color_blend_state_create_info.blendConstants[2] = 0.0f;
    color_blend_state_create_info.blendConstants[3] = 0.0f;

    return color_blend_state_create_info;
}

// MODIFIES: render_pass
// EFFECTS: Creates a render pass for graphics pipeline creation
static void create_render_pass(VkDevice logical_device, VkFormat swap_chain_image_format, VkRenderPass& render_pass) {
    VkAttachmentDescription attachment_description{};
    attachment_description.format = swap_chain_image_format;
    attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference attachment_reference{};
    attachment_reference.attachment = 0;
    attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description{};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &attachment_reference;

    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.pNext = nullptr;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &attachment_description;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_description;
    
    if (vkCreateRenderPass(logical_device, &render_pass_create_info, nullptr, &render_pass) != VK_SUCCESS) {
        throw std::runtime_error("create_render_pass(): Failed to create render pass!");
    }
}

// MODIFIES: pipeline_layout, render_pass, graphics_pipeline
// EFFECTS: Creates graphics pipeline and returns pipeline_layout, render_pass, and graphics_pipeline
void create_graphics_pipeline(
    VkDevice logical_device, 
    VkExtent2D swap_chain_extent, VkFormat swap_chain_image_format, 
    VkRenderPass& render_pass, VkDescriptorSetLayout descriptor_set_layout, 
    VkPipelineLayout& pipeline_layout, VkPipeline& graphics_pipeline
) {
    std::vector<char> vert_shader_code = read_file("./../assets/shaders/shader_vert.spv");
    std::vector<char> frag_shader_code = read_file("./../assets/shaders/shader_frag.spv");

    VkShaderModule vert_shader_module = create_shader_module(logical_device, vert_shader_code);
    VkShaderModule frag_shader_module = create_shader_module(logical_device, frag_shader_code);

    const std::vector<VkPipelineShaderStageCreateInfo> shader_stage_create_info = create_shader_stage_info(logical_device, vert_shader_module, frag_shader_module);
    VkVertexInputBindingDescription vertex_binding_description{};
    std::vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions;
    const VkPipelineVertexInputStateCreateInfo vertex_input_create_info = create_vertex_input_state_info(vertex_binding_description, vertex_attribute_descriptions);
    const VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = create_input_assembly_state_info();
    VkViewport viewport{};
    VkRect2D scissor{};
    const VkPipelineViewportStateCreateInfo viewport_state_create_info = create_viewport_state_info(swap_chain_extent, viewport, scissor);
    const VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = create_rasterization_state_info();
    const VkPipelineMultisampleStateCreateInfo multisample_state_create_info = create_multisample_state_info();
    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    const VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = create_color_blend_state_info(color_blend_attachment);

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pNext = nullptr;

    VkDescriptorSetLayout descriptor_set_layouts[] = {descriptor_set_layout};
    pipeline_layout_create_info.setLayoutCount = 1;
    pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts;
    
    VkPushConstantRange push_constant_range{};
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(int);
    pipeline_layout_create_info.pushConstantRangeCount = 1;
    pipeline_layout_create_info.pPushConstantRanges = &push_constant_range;

    if (vkCreatePipelineLayout(logical_device, &pipeline_layout_create_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("create_graphics_pipeline(): Failed to create graphics pipeline layout!");
    }

    create_render_pass(logical_device, swap_chain_image_format, render_pass);

    VkGraphicsPipelineCreateInfo pipeline_create_info{};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.pNext = nullptr;
    pipeline_create_info.stageCount = 2;
    pipeline_create_info.pStages = shader_stage_create_info.data();
    pipeline_create_info.pVertexInputState = &vertex_input_create_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
    pipeline_create_info.pViewportState = &viewport_state_create_info;
    pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    pipeline_create_info.pMultisampleState = &multisample_state_create_info;
    pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    pipeline_create_info.pDynamicState = nullptr;
    pipeline_create_info.layout = pipeline_layout;
    pipeline_create_info.renderPass = render_pass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &graphics_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("create_graphics_pipeline(): Failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(logical_device, vert_shader_module, nullptr);
    vkDestroyShaderModule(logical_device, frag_shader_module, nullptr);
}