#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D tex;

layout(location = 0) in vec2 fragUv;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 texColor = texture(tex, fragUv);
	if(texColor.r == 1.0 && texColor.b == 1.0 && texColor.g == 0) {
	    discard;
	}

	outColor = texColor;
}