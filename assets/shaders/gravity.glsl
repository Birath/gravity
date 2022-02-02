#version 430

layout(local_size_x = 32) in;

layout(std430, binding = 0) readonly buffer position_buffer{
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
  vec3 my_pos = positions[invocation_id].xyz;
  vec3 my_velocity = vec3(0.0);
  // velocities[invocation_id] = vec4(my_pos, 0.0);
  for (int i = 0; i < positions.length(); ++i) {
    if (i == invocation_id) continue;
    vec3 other_pos = positions[i].xyz;
    float other_mass = positions[i].w;
    float dist = distance(my_pos, other_pos);
    if (dist < 0.000001) continue;
    vec3 direction = normalize(other_pos - my_pos);
    // add mass, constant, delta time?
    vec3 force = gravity_constant * other_mass / (dist * dist) * direction * delta_time;
    my_velocity += force;
  }
  velocities[invocation_id] += vec4(my_velocity, 0.0);
  // velocities[1] = vec4(vec3(1.0, 2.0, 3.0), 0.0);
}