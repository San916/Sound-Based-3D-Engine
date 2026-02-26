cmake_minimum_required(VERSION 4.2.1)

set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
project(Engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(SOURCES
    src/main.cpp
)

add_executable(Engine ${SOURCES})

target_link_libraries(Engine
    PRIVATE
        Vulkan::Vulkan
        glfw
        glm::glm
)