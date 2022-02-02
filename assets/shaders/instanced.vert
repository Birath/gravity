#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in mat4 instance_model;

struct Matrices {
    vec3 world_pos;
    mat4 projection;
    mat4 view;
    mat4 model;

    mat4 mvp;
    mat4 vp;
};

uniform Matrices m;

out vec3 frag_position;
out vec3 frag_normal;
out vec2 frag_uv;

void main(void)
{
	gl_Position = m.vp * instance_model * vec4(position, 1.0);
	// gl_Position = m.mvp * vec4(position, 1.0);
    
	frag_position = (instance_model * vec4(position, 1.0)).xyz;
    frag_normal = normalize(mat3(instance_model) * normal);
    frag_uv = uv;
}