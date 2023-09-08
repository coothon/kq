#version 460 core

layout(binding = 0) uniform UniformBufferObject {
	float time;
	float time_sin;
	float time_cos;
}
kq_uniforms;

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 out_color;

void main(void) {
	out_color = vec4(sin(uv.x + kq_uniforms.time) * 0.5 + 0.5,
	                 cos(uv.y + kq_uniforms.time) * 0.5 + 0.5,
	                 sin(uv.x + uv. y + kq_uniforms.time) * 0.5 + 0.5,
	                 1.0);
}
