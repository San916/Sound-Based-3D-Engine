#version 460

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 position;
} ubo;

layout(binding = 1) uniform sampler2D storage_image;

layout(binding = 2) readonly buffer StorageBufferObject {
    mat4 model[16];
    vec4 sound_waves[1024];
    float amplitudes[1024];
    int visible[16];
    int emitting[16];
    int selected_object_index;
    int num_sound_waves;
} ssbo;

layout(location = 1) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(storage_image, 0));
    outColor = texture(storage_image, uv);
}