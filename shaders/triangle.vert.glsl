#version 460 core
#pragma shader_stage(vertex)

layout(location = 0) in vec2 v_position;
layout(location = 1) in vec3 v_color;

layout(location = 0) out vec3 frag_color;

void main(void) {
	gl_Position = vec4(v_position, 0.0, 1.0);
	frag_color = v_color;
}
