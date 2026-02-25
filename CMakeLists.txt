cmake_minimum_required(VERSION 4.2.1)

project(Engine VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(ENV{VULKAN_SDK} "C:\\VulkanSDK\\1.4.341.1")
find_package(Vulkan REQUIRED)

set(SOURCES
    src/main.cpp
)

add_executable(Engine ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${Vulkan_LIBRARIES}
)