#ifndef VULKAN_HANDLE_H
#define VULKAN_HANDLE_H

#include <cstdint>

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vulkan_vertex_buffer.h>

class VulkanHandle {
private:
    GLFWwindow* window;
    
    VkInstance vk_instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice logical_device;

    VkQueue graphics_queue;
    VkQueue present_queue;

    VkSwapchainKHR swap_chain;
    std::vector<VkImage> swap_chain_images;
    std::vector<VkImageView> swap_chain_image_views;
    VkFormat swap_chain_image_format;
    VkExtent2D swap_chain_extent;

    std::vector<VkFramebuffer> frame_buffers;

    VkDescriptorPool graphics_descriptor_pool;
    std::vector<VkDescriptorSet> graphics_descriptor_sets;
    VkDescriptorSetLayout graphics_descriptor_set_layout;
    VkRenderPass render_pass;
    VkPipelineLayout graphics_pipeline_layout;
    VkPipeline graphics_pipeline;

    VkDescriptorPool compute_descriptor_pool;
    std::vector<VkDescriptorSet> compute_descriptor_sets;
    VkDescriptorSetLayout compute_descriptor_set_layout;
    VkPipelineLayout compute_pipeline_layout;
    VkPipeline compute_pipeline;

    const VkExtent2D dispatch_group_size = {16, 16};

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;

    // Hardcoded scene geometry
    std::vector<Vertex> vertices = {
        // Floor
        {{-2.0f, -1.0f, -5.0f}},
        {{ 2.0f, -1.0f, -5.0f}},
        {{ 2.0f, -1.0f, 4.0f}},
        {{-2.0f, -1.0f, 4.0f}},

        // Ceiling
        {{-2.0f,  1.0f, -5.0f}},
        {{ 2.0f,  1.0f, -5.0f}},
        {{ 2.0f,  1.0f, 4.0f}},
        {{-2.0f,  1.0f, 4.0f}},

        // Left wall
        {{-2.0f, -1.0f, -5.0f}},
        {{-2.0f,  1.0f, -5.0f}},
        {{-2.0f,  1.0f, 4.0f}},
        {{-2.0f, -1.0f, 4.0f}},

        // Right wall
        {{ 2.0f, -1.0f, -5.0f}},
        {{ 2.0f,  1.0f, -5.0f}},
        {{ 2.0f,  1.0f, 4.0f}},
        {{ 2.0f, -1.0f, 4.0f}},

        // Back wall
        {{-2.0f, -1.0f, 2.0f}},
        {{ 2.0f, -1.0f, 2.0f}},
        {{ 2.0f,  1.0f, 2.0f}},
        {{-2.0f,  1.0f, 2.0f}},
    };
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;

    std::vector<uint32_t> indices = {
        // Floor
        0, 1, 2,
        0, 2, 3,
        // Ceiling
        4, 6, 5,
        4, 7, 6,
        // Left wall
        8, 10, 9,
        8, 11, 10,
        // Right wall
        12, 13, 14,
        12, 14, 15,
        // Back wall
        16, 17, 18, 
        16, 18, 19,
    };
    VkBuffer index_buffer;
    VkDeviceMemory index_buffer_memory;

    VkBuffer blas_buffer;
    VkDeviceMemory blas_buffer_memory;
    VkAccelerationStructureKHR blas;

    VkBuffer tlas_buffer;
    VkDeviceMemory tlas_buffer_memory;
    VkAccelerationStructureKHR tlas;

    VkImage storage_image;
    VkDeviceMemory storage_image_memory;
    VkImageView storage_image_view;
    VkFormat storage_image_format;
    VkSampler storage_image_sampler;

    std::vector<VkBuffer> uniform_buffers;
    std::vector<VkDeviceMemory> uniform_buffers_memory;
    std::vector<void*> uniform_buffers_mapped;

    size_t frame_index = 0;
    std::vector<VkSemaphore> acquire_semaphores;
    std::vector<VkSemaphore> render_semaphores;
    std::vector<VkFence> frame_fences;

    void init_vulkan();
    void create_sync_objects();
    void draw_frame();
public:
    VulkanHandle();
    VulkanHandle(const VulkanHandle&) = delete;
    VulkanHandle& operator=(const VulkanHandle&) = delete;
    ~VulkanHandle();

    void run();
};

#endif