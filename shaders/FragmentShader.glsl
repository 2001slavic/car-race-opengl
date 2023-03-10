#version 330

// Input
in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_coordinate;
in vec3 frag_color;

// Output
layout(location = 0) out vec4 out_color;


void main()
{
    out_color = vec4(frag_color, 1);
}
