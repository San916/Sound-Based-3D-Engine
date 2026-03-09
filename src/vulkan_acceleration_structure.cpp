#include <iostream>
#include <cstdint>
#include <string>

#include <vector>

#include <vulkan/vulkan.h>

#include <vulkan_acceleration_structure.h>
#include <vulkan_command_buffer.h>
#include <vulkan_utils.h>
#include <vulkan_vertex_buffer.h>

// EFFECTS: Returns a placeholder function pointer of the given function name
// This is used for get at runtime functions that aren't in the set of core vulkan functions
static PFN_vkVoidFunction load_function(VkDevice logical_device, const char* function_name) {
    PFN_vkVoidFunction pfn = vkGetDeviceProcAddr(logical_device, function_name);
    if (pfn == nullptr) {
        throw std::runtime_error("load_function(): Failed to load " + std::string(function_name));
    }

    return pfn;
}

// MODIFIES: address
// EFFECTS: Gets the address of the buffer in logical_device, and returns it as a VkDeviceOrHostAddressConstKHR
static void get_buffer_device_address_const(VkDevice logical_device, VkBuffer buffer, VkDeviceOrHostAddressConstKHR& address) {
    VkBufferDeviceAddressInfo device_address_info{};
    device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    device_address_info.pNext = nullptr;
    device_address_info.buffer = buffer;

    address.deviceAddress = vkGetBufferDeviceAddress(logical_device, &device_address_info);
}

// MODIFIES: address
// EFFECTS: Gets the address of the buffer in logical_device, and returns it as a VkDeviceOrHostAddressKHR
static void get_buffer_device_address(VkDevice logical_device, VkBuffer buffer, VkDeviceOrHostAddressKHR& address) {
    VkBufferDeviceAddressInfo device_address_info{};
    device_address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    device_address_info.pNext = nullptr;
    device_address_info.buffer = buffer;

    address.deviceAddress = vkGetBufferDeviceAddress(logical_device, &device_address_info);
}

// MODIFIES: as_buffer, as_buffer_memory, as, build_geometry_info
// EFFECTS: 
//     Given the build_geometry_info and max_primitive count, it finds the build_size_info of the as
//     Uses the build_size_info to allocate space for the acceleration structure on the device
//     Binds acceleration structure handle onto the allocated as_buffer
//     Creates a scratch buffer to prepare for the actual building of the acceleration structure
//     Builds the acceleration structure
//   Returns as_buffer_memory, as_buffer, and the handle acceleration_structure
static void create_acceleration_structure(
    VkDevice logical_device, VkPhysicalDevice physical_device, 
    VkCommandPool command_pool, VkQueue graphics_queue, 
    VkAccelerationStructureBuildGeometryInfoKHR& build_geometry_info, uint32_t max_primitive_count, 
    VkBuffer& as_buffer, VkDeviceMemory& as_buffer_memory, VkAccelerationStructureKHR& acceleration_structure
) {
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = 
        (PFN_vkGetAccelerationStructureBuildSizesKHR)load_function(logical_device, "vkGetAccelerationStructureBuildSizesKHR");
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = 
        (PFN_vkCreateAccelerationStructureKHR)load_function(logical_device, "vkCreateAccelerationStructureKHR");
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = 
        (PFN_vkCmdBuildAccelerationStructuresKHR)load_function(logical_device, "vkCmdBuildAccelerationStructuresKHR");

    VkAccelerationStructureBuildSizesInfoKHR build_size_info{};
    build_size_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    vkGetAccelerationStructureBuildSizesKHR(
        logical_device, 
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, 
        &build_geometry_info,
        &max_primitive_count,
        &build_size_info
    );

    create_buffer(
        logical_device, 
        physical_device, 
        build_size_info.accelerationStructureSize, 
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        as_buffer, 
        as_buffer_memory
    );

    VkAccelerationStructureCreateInfoKHR acceleration_structure_create_info{};
    acceleration_structure_create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    acceleration_structure_create_info.pNext = nullptr;
    acceleration_structure_create_info.buffer = as_buffer;
    acceleration_structure_create_info.offset = 0;
    acceleration_structure_create_info.size = build_size_info.accelerationStructureSize;
    acceleration_structure_create_info.type = build_geometry_info.type;
    if (vkCreateAccelerationStructureKHR(logical_device, &acceleration_structure_create_info, nullptr, &acceleration_structure) != VK_SUCCESS) {
        throw std::runtime_error("create_acceleration_structure(): Failed to create acceleration structure!");
    }

    VkDeviceSize scratch_buffer_size = build_size_info.buildScratchSize;
    VkBuffer scratch_buffer;
    VkDeviceMemory stratch_buffer_memory;
    create_buffer(
        logical_device, 
        physical_device, 
        scratch_buffer_size, 
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        scratch_buffer, 
        stratch_buffer_memory
    );


    VkDeviceOrHostAddressKHR scratch_buffer_address;
    get_buffer_device_address(logical_device, scratch_buffer, scratch_buffer_address);

    build_geometry_info.dstAccelerationStructure = acceleration_structure;
    build_geometry_info.scratchData = scratch_buffer_address;

    VkAccelerationStructureBuildRangeInfoKHR build_range_info{};
    build_range_info.primitiveCount = max_primitive_count;
    build_range_info.primitiveOffset = 0;
    build_range_info.firstVertex = 0;
    build_range_info.transformOffset = 0;

    const VkAccelerationStructureBuildRangeInfoKHR* build_range_info_const = &build_range_info;

    VkCommandBuffer command_buffer;
    begin_single_time_command(logical_device, command_pool, command_buffer);

    vkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &build_geometry_info, &build_range_info_const);
    
    finish_single_time_command(logical_device, graphics_queue, command_pool, command_buffer);

    vkDestroyBuffer(logical_device, scratch_buffer, nullptr);
    vkFreeMemory(logical_device, stratch_buffer_memory, nullptr);
}

// MODIFIES: blas_buffer, blas_memory, blas
// EFFECTS: 
//     Creates a blas instance buffer in device memorym
//     Uses vertex/index buffer device addresses to create build_geometry_info, which is used to build the blas in device memory
//   Returns blas_buffer_memory, blas_buffer, and the handle blas
void create_bottom_level_acceleration_structure(
    VkDevice logical_device, VkPhysicalDevice physical_device, 
    VkCommandPool command_pool, VkQueue graphics_queue, 
    VkBuffer vertex_buffer, VkBuffer index_buffer, 
    const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, 
    VkBuffer& blas_buffer, VkDeviceMemory& blas_buffer_memory, VkAccelerationStructureKHR& blas
) {
    VkDeviceOrHostAddressConstKHR vertices_address;
    get_buffer_device_address_const(logical_device, vertex_buffer, vertices_address);

    VkDeviceOrHostAddressConstKHR indices_address;
    get_buffer_device_address_const(logical_device, index_buffer, indices_address);


    VkAccelerationStructureGeometryTrianglesDataKHR triangle_data{};
    triangle_data.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    triangle_data.pNext = nullptr;
    triangle_data.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    triangle_data.vertexData = vertices_address;
    triangle_data.vertexStride = sizeof(Vertex);
    triangle_data.maxVertex = vertices.size() - 1;
    triangle_data.indexType = VK_INDEX_TYPE_UINT32;
    triangle_data.indexData = indices_address;

    VkAccelerationStructureGeometryKHR acceleration_geometry{};
    acceleration_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    acceleration_geometry.pNext = nullptr;
    acceleration_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    acceleration_geometry.geometry.triangles = triangle_data;
    acceleration_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR build_geometry_info{};
    build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    build_geometry_info.pNext = nullptr;
    build_geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    build_geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    build_geometry_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_geometry_info.geometryCount = 1;
    build_geometry_info.pGeometries = &acceleration_geometry;


    uint32_t max_primitive_count = indices.size() / 3;
    create_acceleration_structure(
        logical_device, physical_device, 
        command_pool, graphics_queue, 
        build_geometry_info, max_primitive_count, 
        blas_buffer, blas_buffer_memory, blas
    );
}

// MODIFIES: instance_buffer, instance_buffer_memory
// EFFECTS: Given the tlas_instance, write it into device memory, returning instance_buffer and instance_buffer_memory
static void create_instance_buffer(
    VkDevice logical_device, VkPhysicalDevice physical_device, 
    VkCommandPool command_pool, VkQueue graphics_queue, 
    const VkAccelerationStructureInstanceKHR& tlas_instance, 
    VkBuffer& instance_buffer, VkDeviceMemory& instance_buffer_memory
) {
    VkDeviceSize buffer_size = sizeof(VkAccelerationStructureInstanceKHR);
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    create_buffer(
        logical_device, 
        physical_device, 
        buffer_size, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        staging_buffer, 
        staging_buffer_memory
    );
    void* data;
    vkMapMemory(logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, &tlas_instance, (size_t)buffer_size);
    vkUnmapMemory(logical_device, staging_buffer_memory);

    create_buffer(
        logical_device, 
        physical_device, 
        buffer_size, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
        VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        instance_buffer, 
        instance_buffer_memory
    );
    copy_buffer(logical_device, command_pool, graphics_queue, staging_buffer, instance_buffer, buffer_size);
    vkDestroyBuffer(logical_device, staging_buffer, nullptr);
    vkFreeMemory(logical_device, staging_buffer_memory, nullptr);
}

// MODIFIES: tlas_instance
// EFFECTS: Sets all the fields in tlas_instance, used to create the instance buffer
// This instance will reference the provided bottom level acceleration structure
static void create_tlas_instance(VkDevice logical_device, const VkAccelerationStructureKHR& blas, VkAccelerationStructureInstanceKHR& tlas_instance) {
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR =
        (PFN_vkGetAccelerationStructureDeviceAddressKHR)load_function(logical_device, "vkGetAccelerationStructureDeviceAddressKHR");

    VkAccelerationStructureDeviceAddressInfoKHR blas_address_info{};
    blas_address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    blas_address_info.pNext = nullptr;
    blas_address_info.accelerationStructure = blas;
    VkDeviceAddress blas_address = vkGetAccelerationStructureDeviceAddressKHR(logical_device, &blas_address_info);

    VkTransformMatrixKHR transform = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f
    };

    tlas_instance = VkAccelerationStructureInstanceKHR{};
    tlas_instance.transform = transform;
    tlas_instance.instanceCustomIndex = 0;
    tlas_instance.mask = 0xFF;
    tlas_instance.instanceShaderBindingTableRecordOffset = 0;
    tlas_instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    tlas_instance.accelerationStructureReference = blas_address;
}

// MODIFIES: tlas_buffer, tlas_buffer_memory, tlas
// EFFECTS: 
//     Creates a tlas instance buffer in device memory
//     Uses tlas_instance to create build_geometry_info, which is used to build the tlas in device memory
//   Returns tlas_buffer_memory, tlas_buffer, and the handle tlas
void create_top_level_acceleration_structure(
    VkDevice logical_device, VkPhysicalDevice physical_device, 
    VkCommandPool command_pool, VkQueue graphics_queue, 
    const VkAccelerationStructureKHR& blas, 
    VkBuffer& tlas_buffer, VkDeviceMemory& tlas_buffer_memory, VkAccelerationStructureKHR& tlas
) {
    VkAccelerationStructureInstanceKHR tlas_instance{};
    create_tlas_instance(logical_device, blas, tlas_instance);

    VkBuffer instance_buffer;
    VkDeviceMemory instance_buffer_memory;

    create_instance_buffer(
        logical_device, physical_device, 
        command_pool, graphics_queue, 
        tlas_instance, 
        instance_buffer, instance_buffer_memory
    );


    VkDeviceOrHostAddressConstKHR instance_buffer_address;
    get_buffer_device_address_const(logical_device, instance_buffer, instance_buffer_address);

    VkAccelerationStructureGeometryInstancesDataKHR instance_data{};
    instance_data.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    instance_data.pNext = nullptr;
    instance_data.arrayOfPointers = VK_FALSE;
    instance_data.data = instance_buffer_address;
    
    VkAccelerationStructureGeometryKHR tlas_geometry{};
    tlas_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    tlas_geometry.pNext = nullptr;
    tlas_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    tlas_geometry.geometry.instances = instance_data;
    tlas_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR build_geometry_info{};
    build_geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    build_geometry_info.pNext = nullptr;
    build_geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    build_geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    build_geometry_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_geometry_info.geometryCount = 1;
    build_geometry_info.pGeometries = &tlas_geometry;


    uint32_t max_primitive_count = 1;
    create_acceleration_structure(
        logical_device, physical_device, 
        command_pool, graphics_queue, 
        build_geometry_info, max_primitive_count, 
        tlas_buffer, tlas_buffer_memory, tlas
    );

    vkDestroyBuffer(logical_device, instance_buffer, nullptr);
    vkFreeMemory(logical_device, instance_buffer_memory, nullptr);
}

// MODIFIES: as_buffer, as_buffer_memory, as
// EFFECTS: Cleans up blas_buffer, blas_buffer_memory, and blas
void cleanup_acceleration_structure(VkDevice logical_device, VkBuffer& as_buffer, VkDeviceMemory& as_buffer_memory, VkAccelerationStructureKHR& acceleration_structure) {
    vkDestroyBuffer(logical_device, as_buffer, nullptr);
    vkFreeMemory(logical_device, as_buffer_memory, nullptr);

    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR = 
        (PFN_vkDestroyAccelerationStructureKHR)load_function(logical_device, "vkDestroyAccelerationStructureKHR");
    vkDestroyAccelerationStructureKHR(logical_device, acceleration_structure, nullptr);
}