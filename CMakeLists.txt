cmake_minimum_required(VERSION 4.2.1)

set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
project(Engine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(VERT_SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/shader.vert)
set(FRAG_SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/shader.frag)
set(COMP_SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/shader.comp)
set(VERT_SPV_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/shader_vert.spv)
set(FRAG_SPV_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/shader_frag.spv)
set(COMP_SPV_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/shader_comp.spv)

add_custom_command(
    OUTPUT ${VERT_SPV_OUTPUT}
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${VERT_SHADER_SRC} -o ${VERT_SPV_OUTPUT}
    DEPENDS ${VERT_SHADER_SRC}
)
add_custom_command(
    OUTPUT ${FRAG_SPV_OUTPUT}
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${FRAG_SHADER_SRC} -o ${FRAG_SPV_OUTPUT}
    DEPENDS ${FRAG_SHADER_SRC}
)
add_custom_command(
    OUTPUT ${COMP_SPV_OUTPUT}
    COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${COMP_SHADER_SRC} -o ${COMP_SPV_OUTPUT}
    DEPENDS ${COMP_SHADER_SRC}
)

add_custom_target(compile_shaders ALL DEPENDS ${VERT_SPV_OUTPUT} ${FRAG_SPV_OUTPUT} ${COMP_SPV_OUTPUT})

set(SOURCES
    src/main.cpp
    src/vulkan_acceleration_structure.cpp
    src/vulkan_command_buffer.cpp
    src/vulkan_compute_pipeline.cpp
    src/vulkan_descriptor_sets.cpp
    src/vulkan_frame_buffer.cpp
    src/vulkan_graphics_pipeline.cpp
    src/vulkan_handle.cpp
    src/vulkan_index_buffer.cpp
    src/vulkan_logical_device.cpp
    src/vulkan_physical_device.cpp
    src/vulkan_storage_image.cpp
    src/vulkan_swap_chain.cpp
    src/vulkan_uniform_buffer.cpp
    src/vulkan_utils.cpp
    src/vulkan_validation_layers.cpp
    src/vulkan_vertex_buffer.cpp
    src/vulkan_window.cpp
)

set(INCLUDES
    include
)

add_executable(Engine ${SOURCES})
add_dependencies(Engine compile_shaders)

target_include_directories(Engine
    PRIVATE        
        ${INCLUDES}
)

target_link_libraries(Engine
    PRIVATE
        Vulkan::Vulkan
        glfw
        glm::glm
)