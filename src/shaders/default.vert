#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform MVP
{
    mat4 data;
} mvp;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec2 inUv;

layout(location = 0) out vec2 fragUv;

void main() {
    gl_Position = mvp.data * inPosition;
    fragUv = inUv;
}