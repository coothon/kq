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

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 frag_color;

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) out vec4 out_color;

void main(void) {
	out_color = texture(tex_sampler, uv);
	/*
	out_color = vec4(sin(uv.x + kq_uniforms.time) * 0.5 + 0.5,
	                 cos(uv.y + kq_uniforms.time) * 0.5 + 0.5,
	                 sin(uv.x + uv.y + kq_uniforms.time) * 0.5 + 0.5,
	                 1.0);
	*/
}
