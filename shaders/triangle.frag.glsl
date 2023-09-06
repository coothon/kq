#version 460 core
#pragma shader_stage(fragment)

layout(binding = 0) uniform UniformBufferObject {
	float time;
	float time_sin;
	float time_cos;
} kq_uniforms;

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 out_color;

void main(void) {
	out_color = vec4((sin(uv.x + kq_uniforms.time)        + 1.0) / 2.0,
	                 (cos(uv.y + kq_uniforms.time)        + 1.0) / 2.0,
	                 (cos(uv.x + uv.y + kq_uniforms.time) + 1.0) / 2.0,
			 1.0);
}
