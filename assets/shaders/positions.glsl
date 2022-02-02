#version 430

layout(local_size_x = 32) in;

layout(std430, binding = 0) buffer position_buffer{
  vec4 positions[];
};

layout(std430, binding = 1) buffer velocity_buffer{
  vec4 velocities[];
};

uniform float delta_time;
uniform float gravity_constant;


void main()
{
  int invocation_id = int(gl_GlobalInvocationID.x);
  // positions[invocation_id].xyz += vec3(0.5725, 0.0431, 0.0431) * delta_time;
  positions[invocation_id].xyz += vec3(velocities[invocation_id]) * delta_time;
}