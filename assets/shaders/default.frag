#version 330
in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_uv;

out vec4 out_color;

void main(void)
{
    out_color = vec4(vec3(normalize(abs(frag_position))), 1);
}
