#version 460 core

// Uniforms.
layout(binding = 0) restrict readonly uniform UniformBufferObject {
	restrict readonly float time;
	restrict readonly float time_sin;
	restrict readonly float time_cos;
} kq_uniforms;


// Push constants.
layout(push_constant, std430) restrict readonly uniform pc {
	layout(offset = 0) restrict readonly vec2 position;
	layout(offset = 8) restrict readonly vec2 scale;
	layout(offset = 16) restrict readonly float tiles_tex_index;
};


// Inputs.
layout(location = 0) in vec2 v_position;
layout(location = 1) in vec2 v_uv;


// Outputs.
layout(location = 0) out vec2 uv;


void main(void) {
	gl_Position = vec4(v_position * scale + position, 0.0, 1.0);
	uv = v_uv;
}
