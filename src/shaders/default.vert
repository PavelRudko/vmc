#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform MVP
{
    mat4 data;
} mvp;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec2 inUv;

layout(location = 0) out vec2 fragUv;
layout(location = 1) out float illuminance;

void main() {
    gl_Position = mvp.data * vec4(inPosition.xyz, 1.0);
    fragUv = inUv;
	illuminance = inPosition.w;
}