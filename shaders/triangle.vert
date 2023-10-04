#version 460 core

layout(binding = 0) uniform UniformBufferObject {
	float time;
	float time_sin;
	float time_cos;
}
kq_uniforms;

layout(push_constant) uniform pc {
	layout(offset = 0) vec2 position;
	layout(offset = 8) vec2 scale;
};

layout(location = 0) in vec2 v_position;
layout(location = 1) in vec2 v_uv;
layout(location = 2) in vec3 v_color;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 frag_color;

void main(void) {
	gl_Position = vec4(v_position * scale + position, 0.0, 1.0);
	uv          = v_uv;
	frag_color  = v_color;
}
