#include <iostream>

#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan_descriptor_sets.h>
#include <vulkan_storage_buffer.h>
#include <vulkan_uniform_buffer.h>
#include <vulkan_utils.h>

// MODIFIES: descriptor_set_layout
// EFFECTS: Uses the given descriptor set layout bindings to create a descriptor set layout
static void create_descriptor_set_layout(
    VkDevice logical_device, 
    const std::vector<VkDescriptorSetLayoutBinding>& bindings, 
    VkDescriptorSetLayout& descriptor_set_layout
) {
    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.pNext = nullptr;
    descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptor_set_layout_create_info.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logical_device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
        throw std::runtime_error("create_descriptor_set_layout(): Failed to create descriptor set layout!");
    }
}

// MODIFIES: descriptor_sets
// EFFECTS: Allocates descriptor sets in the descriptor pool
static void allocate_descriptor_sets(
    VkDevice logical_device, size_t max_frames_in_flight, 
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
        throw std::runtime_error("allocate_descriptor_sets(): Failed to allocate descriptor sets!");
    }
}

// MODIFIES: graphics_descriptor_set_layout
// EFFECTS: Creates a descriptor set layout for the graphics pipeline and returns it
//     Binds: UBO, storage image
void create_graphics_descriptor_set_layout(VkDevice logical_device, VkDescriptorSetLayout& graphics_descriptor_set_layout) {
    VkDescriptorSetLayoutBinding uniform_buffer_binding{};
    uniform_buffer_binding.binding = 0;
    uniform_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_buffer_binding.descriptorCount = 1;
    uniform_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uniform_buffer_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding storage_image_binding{};
    storage_image_binding.binding = 1;
    storage_image_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    storage_image_binding.descriptorCount = 1;
    storage_image_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    storage_image_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding storage_buffer_binding{};
    storage_buffer_binding.binding = 2;
    storage_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storage_buffer_binding.descriptorCount = 1;
    storage_buffer_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    storage_buffer_binding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        uniform_buffer_binding, storage_image_binding, storage_buffer_binding
    };

    create_descriptor_set_layout(logical_device, bindings, graphics_descriptor_set_layout);
}

// MODIFIES: graphics_descriptor_pool
// EFFECTS: Creates the graphics descriptor pool with entries equal to the number of inflight frames
void create_graphics_descriptor_pool(VkDevice logical_device, size_t max_frames_in_flight, VkDescriptorPool& graphics_descriptor_pool) {
    VkDescriptorPoolSize uniform_buffer_size{};
    uniform_buffer_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_buffer_size.descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

    VkDescriptorPoolSize storage_image_sample_size{};
    storage_image_sample_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    storage_image_sample_size.descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

    VkDescriptorPoolSize storage_buffer_size{};
    storage_buffer_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storage_buffer_size.descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

    std::vector<VkDescriptorPoolSize> pool_sizes = {uniform_buffer_size, storage_image_sample_size, storage_buffer_size};

    VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.pNext = nullptr;
    descriptor_pool_create_info.maxSets = static_cast<uint32_t>(max_frames_in_flight);
    descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    descriptor_pool_create_info.pPoolSizes = pool_sizes.data();

    if (vkCreateDescriptorPool(logical_device, &descriptor_pool_create_info, nullptr, &graphics_descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("create_graphics_descriptor_pool(): Failed to create graphics descriptor pool!");
    }
}

// REQUIRES: uniform_buffers.size(), descriptor_set_layout.size() >= max_frames_in_flight
// MODIFIES: descriptor_sets
// EFFECTS: Allocates descriptor sets and writes the resources into the descriptor sets, with unique bindings
//     Writes the uniform buffer in binding 0 of each descriptor set
//     Writes the storage image as a sampler (read only) in binding 1 of each descriptor set
//     Writes the storage buffer in binding 2
void create_graphics_descriptor_sets(
    VkDevice logical_device, 
    size_t max_frames_in_flight, 
    VkDescriptorPool graphics_descriptor_pool, 
    VkSampler& storage_image_sampler, 
    const VkImageView storage_image_view, 
    const std::vector<VkBuffer>& uniform_buffers, 
    const std::vector<VkBuffer>& storage_buffers,
    const VkDescriptorSetLayout& graphics_descriptor_set_layout, 
    std::vector<VkDescriptorSet>& graphics_descriptor_sets
) {
    allocate_descriptor_sets(logical_device, max_frames_in_flight, graphics_descriptor_pool, graphics_descriptor_set_layout, graphics_descriptor_sets);

    VkDescriptorBufferInfo uniform_buffer_info{};
    uniform_buffer_info.buffer = nullptr;
    uniform_buffer_info.offset = 0;
    uniform_buffer_info.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet uniform_buffer_write{};
    uniform_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    uniform_buffer_write.pNext = nullptr;
    uniform_buffer_write.dstBinding = 0;
    uniform_buffer_write.dstArrayElement = 0;
    uniform_buffer_write.descriptorCount = 1;
    uniform_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_buffer_write.pImageInfo = nullptr;
    uniform_buffer_write.pBufferInfo = &uniform_buffer_info;
    uniform_buffer_write.pTexelBufferView = nullptr;

    VkSamplerCreateInfo storage_image_sampler_create_info{};
    storage_image_sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    storage_image_sampler_create_info.magFilter = VK_FILTER_NEAREST;
    storage_image_sampler_create_info.minFilter = VK_FILTER_NEAREST;
    storage_image_sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    storage_image_sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    storage_image_sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    storage_image_sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    storage_image_sampler_create_info.unnormalizedCoordinates = VK_FALSE;

    if (vkCreateSampler(logical_device, &storage_image_sampler_create_info, nullptr, &storage_image_sampler) != VK_SUCCESS) {
        throw std::runtime_error("create_graphics_descriptor_sets(): Failed to create storage image sampler!");
    }

    VkDescriptorImageInfo storage_image_info{};
    storage_image_info.sampler = storage_image_sampler;
    storage_image_info.imageView = storage_image_view;
    storage_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet storage_image_write{};
    storage_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    storage_image_write.pNext = nullptr;
    storage_image_write.dstBinding = 1;
    storage_image_write.dstArrayElement = 0;
    storage_image_write.descriptorCount = 1;
    storage_image_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    storage_image_write.pImageInfo = &storage_image_info;
    storage_image_write.pBufferInfo = nullptr;
    storage_image_write.pTexelBufferView = nullptr;

    VkDescriptorBufferInfo storage_buffer_info{};
    storage_buffer_info.buffer = nullptr;
    storage_buffer_info.offset = 0;
    storage_buffer_info.range = sizeof(StorageBufferObject);

    VkWriteDescriptorSet storage_buffer_write{};
    storage_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    storage_buffer_write.pNext = nullptr;
    storage_buffer_write.dstBinding = 2;
    storage_buffer_write.dstArrayElement = 0;
    storage_buffer_write.descriptorCount = 1;
    storage_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storage_buffer_write.pImageInfo = nullptr;
    storage_buffer_write.pBufferInfo = &storage_buffer_info;
    storage_buffer_write.pTexelBufferView = nullptr;

    for (size_t i = 0; i < max_frames_in_flight; i++) {
        uniform_buffer_info.buffer = uniform_buffers[i];
        uniform_buffer_write.dstSet = graphics_descriptor_sets[i];

        storage_image_write.dstSet = graphics_descriptor_sets[i];

        storage_buffer_info.buffer = storage_buffers[i];
        storage_buffer_write.dstSet = graphics_descriptor_sets[i];

        std::vector<VkWriteDescriptorSet> writes = {uniform_buffer_write, storage_image_write, storage_buffer_write};

        vkUpdateDescriptorSets(logical_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }
}


// MODIFIES: compute_descriptor_set_layout
// EFFECTS: Creates a descriptor set layout for the compute pipeline and returns it
//     Binds: TLAS, storage image, UBO, storage buffer, object id image
void create_compute_descriptor_set_layout(VkDevice logical_device, VkDescriptorSetLayout& compute_descriptor_set_layout) {
    VkDescriptorSetLayoutBinding tlas_binding{};
    tlas_binding.binding = 0;
    tlas_binding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    tlas_binding.descriptorCount = 1;
    tlas_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    tlas_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding storage_image_binding{};
    storage_image_binding.binding = 1;
    storage_image_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storage_image_binding.descriptorCount = 1;
    storage_image_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    storage_image_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding uniform_buffer_binding{};
    uniform_buffer_binding.binding = 2;
    uniform_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_buffer_binding.descriptorCount = 1;
    uniform_buffer_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    uniform_buffer_binding.pImmutableSamplers = nullptr;
    
    VkDescriptorSetLayoutBinding storage_buffer_binding{};
    storage_buffer_binding.binding = 3;
    storage_buffer_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storage_buffer_binding.descriptorCount = 1;
    storage_buffer_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    storage_buffer_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding object_id_image_binding{};
    object_id_image_binding.binding = 4;
    object_id_image_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    object_id_image_binding.descriptorCount = 1;
    object_id_image_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    object_id_image_binding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        tlas_binding, storage_image_binding, uniform_buffer_binding, storage_buffer_binding, object_id_image_binding
    };

    create_descriptor_set_layout(logical_device, bindings, compute_descriptor_set_layout);
}

// MODIFIES: compute_descriptor_pool
// EFFECTS: Creates the compute descriptor pool with entries equal to the number of inflight frames
void create_compute_descriptor_pool(VkDevice logical_device, size_t max_frames_in_flight, VkDescriptorPool& compute_descriptor_pool) {
    VkDescriptorPoolSize tlas_size{};
    tlas_size.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    tlas_size.descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

    VkDescriptorPoolSize storage_image_size{};
    storage_image_size.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storage_image_size.descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

    VkDescriptorPoolSize uniform_buffer_size{};
    uniform_buffer_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_buffer_size.descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

    VkDescriptorPoolSize storage_buffer_size{};
    storage_buffer_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storage_buffer_size.descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

    VkDescriptorPoolSize object_id_image_size{};
    object_id_image_size.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    object_id_image_size.descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

    std::vector<VkDescriptorPoolSize> pool_sizes = {tlas_size, storage_image_size, uniform_buffer_size, storage_buffer_size, object_id_image_size};

    VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.pNext = nullptr;
    descriptor_pool_create_info.maxSets = static_cast<uint32_t>(max_frames_in_flight);
    descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    descriptor_pool_create_info.pPoolSizes = pool_sizes.data();

    if (vkCreateDescriptorPool(logical_device, &descriptor_pool_create_info, nullptr, &compute_descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("create_compute_descriptor_pool(): Failed to create compute descriptor pool!");
    }
}

// REQUIRES: compute_descriptor_set_layout.size() >= max_frames_in_flight
// MODIFIES: compute_descriptor_sets
// EFFECTS: Allocates compute descriptor sets and writes the resources into each descriptor set with unique bindings
//     Writes the top level acceleration structure in binding 0 of each descriptor set
//     Writes the storage image (allows writing into, unlike a sampler) in binding 1 of each descriptor set
//     Writes the ubo in binding 2
//     Writes the storage buffer in binding 3
//     Writes the object id image (allows writing into) in binding 4
void create_compute_descriptor_sets(
    VkDevice logical_device,
    size_t max_frames_in_flight,
    VkDescriptorPool compute_descriptor_pool,
    const VkImageView storage_image_view,
    const VkImageView object_id_image_view,
    const std::vector<VkAccelerationStructureKHR>& tlases,
    const std::vector<VkBuffer>& uniform_buffers,
    const std::vector<VkBuffer>& storage_buffers,
    const VkDescriptorSetLayout& compute_descriptor_set_layout,
    std::vector<VkDescriptorSet>& compute_descriptor_sets
) {
    allocate_descriptor_sets(logical_device, max_frames_in_flight, compute_descriptor_pool, compute_descriptor_set_layout, compute_descriptor_sets);

    VkWriteDescriptorSetAccelerationStructureKHR tlas_write_info = {};
    tlas_write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    tlas_write_info.accelerationStructureCount = 1;
    tlas_write_info.pAccelerationStructures = nullptr;

    VkWriteDescriptorSet tlas_write = {};
    tlas_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    tlas_write.pNext = &tlas_write_info;
    tlas_write.dstBinding = 0;
    tlas_write.dstArrayElement = 0;
    tlas_write.descriptorCount = 1;
    tlas_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    tlas_write.pImageInfo = nullptr;
    tlas_write.pBufferInfo = nullptr;
    tlas_write.pTexelBufferView = nullptr;

    VkDescriptorImageInfo storage_image_info{};
    storage_image_info.sampler = nullptr;
    storage_image_info.imageView = storage_image_view;
    storage_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet storage_image_write{};
    storage_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    storage_image_write.pNext = nullptr;
    storage_image_write.dstBinding = 1;
    storage_image_write.dstArrayElement = 0;
    storage_image_write.descriptorCount = 1;
    storage_image_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storage_image_write.pImageInfo = &storage_image_info;
    storage_image_write.pBufferInfo = nullptr;
    storage_image_write.pTexelBufferView = nullptr;

    VkDescriptorBufferInfo uniform_buffer_info{};
    uniform_buffer_info.buffer = nullptr;
    uniform_buffer_info.offset = 0;
    uniform_buffer_info.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet uniform_buffer_write{};
    uniform_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    uniform_buffer_write.pNext = nullptr;
    uniform_buffer_write.dstBinding = 2;
    uniform_buffer_write.dstArrayElement = 0;
    uniform_buffer_write.descriptorCount = 1;
    uniform_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniform_buffer_write.pImageInfo = nullptr;
    uniform_buffer_write.pBufferInfo = &uniform_buffer_info;
    uniform_buffer_write.pTexelBufferView = nullptr;

    VkDescriptorBufferInfo storage_buffer_info{};
    storage_buffer_info.buffer = nullptr;
    storage_buffer_info.offset = 0;
    storage_buffer_info.range = sizeof(StorageBufferObject);

    VkWriteDescriptorSet storage_buffer_write{};
    storage_buffer_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    storage_buffer_write.pNext = nullptr;
    storage_buffer_write.dstBinding = 3;
    storage_buffer_write.dstArrayElement = 0;
    storage_buffer_write.descriptorCount = 1;
    storage_buffer_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storage_buffer_write.pImageInfo = nullptr;
    storage_buffer_write.pBufferInfo = &storage_buffer_info;
    storage_buffer_write.pTexelBufferView = nullptr;

    VkDescriptorImageInfo object_id_image_info{};
    object_id_image_info.sampler = nullptr;
    object_id_image_info.imageView = object_id_image_view;
    object_id_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet object_id_image_write{};
    object_id_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    object_id_image_write.pNext = nullptr;
    object_id_image_write.dstBinding = 4;
    object_id_image_write.dstArrayElement = 0;
    object_id_image_write.descriptorCount = 1;
    object_id_image_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    object_id_image_write.pImageInfo = &object_id_image_info;
    object_id_image_write.pBufferInfo = nullptr;
    object_id_image_write.pTexelBufferView = nullptr;

    for (size_t i = 0; i < max_frames_in_flight; i++) {
        tlas_write_info.pAccelerationStructures = &tlases[i];
        tlas_write.dstSet = compute_descriptor_sets[i];

        storage_image_write.dstSet = compute_descriptor_sets[i];

        uniform_buffer_info.buffer = uniform_buffers[i];
        uniform_buffer_write.dstSet = compute_descriptor_sets[i];

        storage_buffer_info.buffer = storage_buffers[i];
        storage_buffer_write.dstSet = compute_descriptor_sets[i];

        object_id_image_write.dstSet = compute_descriptor_sets[i];

        std::vector<VkWriteDescriptorSet> writes = {tlas_write, storage_image_write, uniform_buffer_write, storage_buffer_write, object_id_image_write};

        vkUpdateDescriptorSets(logical_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }
}


// MODIFIES: post_process_descriptor_set_layout
// EFFECTS: Creates a descriptor set layout for the post-process pipeline and returns it
//     Binds: storage image, object id image
void create_post_process_descriptor_set_layout(VkDevice logical_device, VkDescriptorSetLayout& post_process_descriptor_set_layout) {
    VkDescriptorSetLayoutBinding storage_image_binding{};
    storage_image_binding.binding = 0;
    storage_image_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storage_image_binding.descriptorCount = 1;
    storage_image_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    storage_image_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding object_id_image_binding{};
    object_id_image_binding.binding = 1;
    object_id_image_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    object_id_image_binding.descriptorCount = 1;
    object_id_image_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    object_id_image_binding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        storage_image_binding, object_id_image_binding
    };

    create_descriptor_set_layout(logical_device, bindings, post_process_descriptor_set_layout);
}

// MODIFIES: post_process_descriptor_pool
// EFFECTS: Creates the post-process descriptor pool with entries equal to the number of inflight frames
void create_post_process_descriptor_pool(VkDevice logical_device, size_t max_frames_in_flight, VkDescriptorPool& post_process_descriptor_pool) {
    VkDescriptorPoolSize storage_image_size{};
    storage_image_size.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storage_image_size.descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

    VkDescriptorPoolSize object_id_image_size{};
    object_id_image_size.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    object_id_image_size.descriptorCount = static_cast<uint32_t>(max_frames_in_flight);

    std::vector<VkDescriptorPoolSize> pool_sizes = {storage_image_size, object_id_image_size};

    VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.pNext = nullptr;
    descriptor_pool_create_info.maxSets = static_cast<uint32_t>(max_frames_in_flight);
    descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    descriptor_pool_create_info.pPoolSizes = pool_sizes.data();

    if (vkCreateDescriptorPool(logical_device, &descriptor_pool_create_info, nullptr, &post_process_descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("create_post_process_descriptor_pool(): Failed to create post-process descriptor pool!");
    }
}

// REQUIRES: post_process_descriptor_set_layout.size() >= max_frames_in_flight
// MODIFIES: post_process_descriptor_sets
// EFFECTS: Allocates post-process descriptor sets and writes the resources into each descriptor set with unique bindings
//     Writes the storage image (allows writing into) in binding 0 of each descriptor set
//     Writes the object id image (allows writing into but we only read, because we cant use sampler for this image format) in binding 1
void create_post_process_descriptor_sets(
    VkDevice logical_device,
    size_t max_frames_in_flight,
    VkDescriptorPool post_process_descriptor_pool,
    const VkImageView storage_image_view,
    const VkImageView object_id_image_view,
    const VkDescriptorSetLayout& post_process_descriptor_set_layout,
    std::vector<VkDescriptorSet>& post_process_descriptor_sets
) {
    allocate_descriptor_sets(logical_device, max_frames_in_flight, post_process_descriptor_pool, post_process_descriptor_set_layout, post_process_descriptor_sets);

    VkDescriptorImageInfo storage_image_info{};
    storage_image_info.sampler = nullptr;
    storage_image_info.imageView = storage_image_view;
    storage_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet storage_image_write{};
    storage_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    storage_image_write.pNext = nullptr;
    storage_image_write.dstBinding = 0;
    storage_image_write.dstArrayElement = 0;
    storage_image_write.descriptorCount = 1;
    storage_image_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storage_image_write.pImageInfo = &storage_image_info;
    storage_image_write.pBufferInfo = nullptr;
    storage_image_write.pTexelBufferView = nullptr;

    VkDescriptorImageInfo object_id_image_info{};
    object_id_image_info.sampler = nullptr;
    object_id_image_info.imageView = object_id_image_view;
    object_id_image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    VkWriteDescriptorSet object_id_image_write{};
    object_id_image_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    object_id_image_write.pNext = nullptr;
    object_id_image_write.dstBinding = 1;
    object_id_image_write.dstArrayElement = 0;
    object_id_image_write.descriptorCount = 1;
    object_id_image_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    object_id_image_write.pImageInfo = &object_id_image_info;
    object_id_image_write.pBufferInfo = nullptr;
    object_id_image_write.pTexelBufferView = nullptr;

    for (size_t i = 0; i < max_frames_in_flight; i++) {
        storage_image_write.dstSet = post_process_descriptor_sets[i];

        object_id_image_write.dstSet = post_process_descriptor_sets[i];

        std::vector<VkWriteDescriptorSet> writes = {storage_image_write, object_id_image_write};

        vkUpdateDescriptorSets(logical_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }
}