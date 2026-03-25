#version 460

layout(push_constant) uniform PushConstants {
    int obj_index;
} push_constants;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 position;
} ubo;

layout(binding = 2) readonly buffer StorageBufferObject {
    mat4 model[16];
    vec4 sound_waves[1024];
    float amplitudes[1024];
    int visible[16];
    int emitting[16];
    int selected_object_index;
} ssbo;

layout(location = 0) in vec3 inPosition;

layout(location = 1) out vec3 frag_color;

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = ubo.proj * ubo.view * ssbo.model[push_constants.obj_index] * vec4(inPosition, 1.0);
    frag_color = colors[1];
}