#ifndef VULKAN_HANDLE_H
#define VULKAN_HANDLE_H

#include <cstdint>

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vulkan_index_buffer.h>
#include <vulkan_scene.h>
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

    Scene* scene;

    std::vector<VkBuffer> tlas_buffer;
    std::vector<VkDeviceMemory> tlas_buffer_memory;
    std::vector<VkAccelerationStructureKHR> tlases;

    VkImage storage_image;
    VkDeviceMemory storage_image_memory;
    VkImageView storage_image_view;
    VkFormat storage_image_format;
    VkSampler storage_image_sampler;

    VkImage object_id_image;
    VkDeviceMemory object_id_image_memory;
    VkImageView object_id_image_view;
    VkFormat object_id_image_format;

    std::vector<VkBuffer> storage_buffers;
    std::vector<VkDeviceMemory> storage_buffers_memory;
    std::vector<void*> storage_buffers_mapped;
    std::vector<glm::vec4> sound_waves;

    std::vector<VkBuffer> uniform_buffers;
    std::vector<VkDeviceMemory> uniform_buffers_memory;
    std::vector<void*> uniform_buffers_mapped;
    glm::vec3 camera_position = {0.0f, 0.0f, 0.0f};
    glm::vec2 camera_rotation = {90.0f, 0.0f};
    glm::vec2 last_mouse_position = {0.0f, 0.0f};
    bool q_held_down = false;
    bool initial_mouse_position_set = false;

    int prev_frame = -1;
    size_t frame_index = 0;
    std::vector<VkSemaphore> acquire_semaphores;
    std::vector<VkSemaphore> render_semaphores;
    std::vector<VkFence> frame_fences;

    void init_vulkan();
    void create_sync_objects();
    void draw_frame();
    static void mouse_callback(GLFWwindow* window, double x_pos, double y_pos);
public:
    VulkanHandle();
    VulkanHandle(const VulkanHandle&) = delete;
    VulkanHandle& operator=(const VulkanHandle&) = delete;
    ~VulkanHandle();

    void run();
};

#endif