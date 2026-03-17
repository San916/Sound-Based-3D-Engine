#version 460

layout(binding = 0) uniform UniformBufferObject {
    mat4 model[16];
    mat4 view;
    mat4 proj;
    vec4 position;
    vec4 sound_waves[16];
} ubo;

layout(binding = 1) uniform sampler2D storage_image;

layout(location = 1) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(storage_image, 0));
    outColor = texture(storage_image, uv);
}