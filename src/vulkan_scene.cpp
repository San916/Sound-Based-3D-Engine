#include <iostream>
#include <cstdint>
#include <fstream>

#include <string>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan_acceleration_structure.h>
#include <vulkan_index_buffer.h>
#include <vulkan_object.h>
#include <vulkan_scene.h>
#include <vulkan_vertex_buffer.h>

// MODIFIES: this->objects
// EFFECTS: Creates a new VulkanObject instance for the given object file, and inserts into objects
void Scene::parse_object_file(const char* buf) {
    char object_file_name[256];
    if (std::sscanf(buf, "o %255s", object_file_name) != 1) {
        throw std::runtime_error("parse_object_file(): Incorrect object format!");
    }
    VulkanObject* new_object = new VulkanObject(object_file_name, logical_device, physical_device, command_pool, graphics_queue);
    objects.push_back(new_object);
}

static glm::vec3 read_x_values(const char* buf, int x) {
    glm::vec3 value;
    if (x == 1 && sscanf(buf, "%f", &value.x) != 1 ||
        x == 2 && sscanf(buf, "%f %f", &value.x, &value.y) != 2 ||
        x == 3 && sscanf(buf, "%f %f %f", &value.x, &value.y, &value.z) != 3) {
        throw std::runtime_error("read_x_values(): Incorrect buf format!");
    }
    return value;
}

// MODIFIES: this->objects
// EFFECTS: parses the object property in buf into the appropriate location in the latest object's ObjectProperties
void Scene::parse_object_property(const char* buf) {
    if (objects.size() == 0) {
        throw std::runtime_error("parse_object_property(): objects.size() must be greater than 0!");
    }
    glm::vec3 value;
    glm::ivec3 value_int;
    
    printf("BUF %s\n", buf);

    if (buf[1] == ' ') {
        switch (buf[0]) {
            case 'p':
                value = read_x_values(buf + 2, 3);
                objects[objects.size() - 1]->properties.position = value;
            case 'r':
                value = read_x_values(buf + 2, 3);
                objects[objects.size() - 1]->properties.rotation = value;
            case 's':
                value = read_x_values(buf + 2, 3);
                objects[objects.size() - 1]->properties.scale = value;
            case 'v':
                value = read_x_values(buf + 2, 1);
                objects[objects.size() - 1]->properties.visible = static_cast<int>(value.x);
            case 'e':
                value = read_x_values(buf + 2, 1);
                objects[objects.size() - 1]->properties.emitting = static_cast<int>(value.x);
            default:
                throw std::runtime_error("parse_object_property(): Incorrect object property format!1");
        }
        return;
    }
    
    if (buf[2] == ' ') {
        switch (buf[0]) {
            case 'p':
                value = read_x_values(buf + 2, 3);
                objects[objects.size() - 1]->properties.position = value;
            case 'r':
                value = read_x_values(buf + 2, 3);
                objects[objects.size() - 1]->properties.rotation = value;
            case 's':
                value = read_x_values(buf + 2, 3);
                objects[objects.size() - 1]->properties.scale = value;
            case 'v':
                value = read_x_values(buf + 2, 1);
                objects[objects.size() - 1]->properties.visible = static_cast<int>(value.x);
            case 'e':
                value = read_x_values(buf + 2, 1);
                objects[objects.size() - 1]->properties.emitting = static_cast<int>(value.x);
            default:
                throw std::runtime_error("parse_object_property(): Incorrect object property format!1");
        }
        return;
    }

    if (std::sscanf(buf, "%*c%*c %d", &value_int.x) == 1) {
        if (buf[0] == 'p' && buf[1] == 'e') objects[objects.size() - 1]->properties.physics_enabled = value_int.x;
        else {
            throw std::runtime_error("parse_object_property(): Incorrect object property format!3");
        }
        return;
    }
    if (std::sscanf(buf, "%*c%*c %f", &value.x) == 1) {
        printf("HERE3\n");
        if (buf[0] == 'p' && buf[1] == 'm') objects[objects.size() - 1]->properties.mass = value.x;
        else {
            throw std::runtime_error("parse_object_property(): Incorrect object property format!2");
        }
        return;
    }
    throw std::runtime_error("parse_object_property(): Incorrect object property format!4");
}

Scene::Scene(
    const std::string& file_name, 
    VkDevice logical_device, 
    VkPhysicalDevice physical_device, 
    VkCommandPool command_pool, 
    VkQueue graphics_queue
) : file_name(file_name) {
    this->logical_device = logical_device;
    this->physical_device = physical_device;
    this->command_pool = command_pool;
    this->graphics_queue = graphics_queue;
    load_scene_file(); 
}

Scene::~Scene() {
    for (VulkanObject* object : objects) delete object;
}

// MODIFIES: this
// EFFECTS: Loads scene file specified at this->file_name
void Scene::load_scene_file() {
    std::ifstream scene_file(file_name, std::ios::ate | std::ios::binary);
    if (scene_file.fail()) {
        throw std::runtime_error("load_scene_file(): Failed to load file!");
    }

    size_t file_size = scene_file.tellg();
    std::string content(file_size, '\0');
    scene_file.seekg(0);
    scene_file.read(content.data(), file_size);
    scene_file.close();

    char* ptr = content.data();
    const char* end = ptr + file_size;

    while (ptr < end) {
        char* line_end = ptr;
        while (line_end < end && *line_end != '\n' && *line_end != '\r') line_end++;

        char end_char = '\0';
        if (line_end < end) {
            end_char = *line_end;
            *line_end = '\0';
        }

        size_t len = line_end - ptr;
        if (len > 1) {
            if (ptr[0] == 'o' && ptr[1] == ' ') {
                parse_object_file(ptr);
            } else if (
                (
                    ptr[0] == 'p' ||
                    ptr[0] == 'r' ||
                    ptr[0] == 's' ||
                    ptr[0] == 'v' ||
                    ptr[0] == 'e'
                ) && ptr[1] == ' ' ||
                ptr[0] == 'p' &&
                (
                    ptr[1] == 'e' ||
                    ptr[1] == 'm'
                ) && ptr[2] == ' '
            ) {
                parse_object_property(ptr);
            }
        }

        if (line_end < end) {
            *line_end = end_char;
        }

        while (line_end < end && (*line_end == '\n' || *line_end == '\r')) line_end++;
        ptr = line_end;
    }
}