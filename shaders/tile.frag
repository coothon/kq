#version 460 core

// Uniforms.
layout(binding = 0) restrict readonly uniform UniformBufferObject {
	restrict readonly float time;
	restrict readonly float time_sin;
	restrict readonly float time_cos;
} kq_uniforms;

layout(binding = 1) uniform sampler2DArray tiles_tex;


// Push constants.
layout(push_constant, std430) restrict readonly uniform pc {
	layout(offset = 0) restrict readonly vec2 position;
	layout(offset = 8) restrict readonly vec2 scale;
	layout(offset = 16) restrict readonly float tiles_tex_index;
};


// Inputs.
layout(location = 0) in vec2 uv;


// Outputs.
layout(location = 0) out vec4 out_color;


void main(void) {
	out_color = texture(tiles_tex, vec3(uv, tiles_tex_index));
}
