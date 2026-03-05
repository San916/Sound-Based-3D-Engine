#ifndef VULKAN_HANDLE_H
#define VULKAN_HANDLE_H

#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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

    VkDescriptorPool descriptor_pool;
    std::vector<VkDescriptorSet> descriptor_sets;
    VkDescriptorSetLayout descriptor_set_layout;
    VkPipelineLayout pipeline_layout;
    VkRenderPass render_pass;
    VkPipeline graphics_pipeline;

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> command_buffers;

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