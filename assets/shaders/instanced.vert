#version 430

layout(std430, binding = 0) buffer position_buffer{
  vec4 positions[];
};

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

struct Matrices {
    mat4 vp;
};

uniform Matrices m;

out vec3 frag_position;
out vec3 frag_normal;
out vec2 frag_uv;

void main(void)
{
    vec4 instance_position = positions[gl_InstanceID];
    mat4 aMat4 = mat4(1.0, 0.0, 0.0, 0,  // 1. column
                      0.0, 1.0, 0.0, 0,  // 2. column
                      0.0, 0.0, 1.0, 0,  // 3. column
                      instance_position.x, instance_position.y, instance_position.z, 1.0);                  // 4. column
	gl_Position = m.vp * aMat4 * vec4(position, 1.0);
    
	frag_position = (aMat4 * vec4(position, 1.0)).xyz;
    frag_position = vec3(normalize(instance_position.xyz));
    frag_normal = normalize(mat3(aMat4) * normal);
    frag_uv = uv;
}