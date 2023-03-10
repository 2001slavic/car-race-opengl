#version 330

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_coordinate;
layout(location = 3) in vec3 v_color;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform vec4 player_screen_pos;

// Output
out vec3 frag_position;
out vec3 frag_normal;
out vec2 frag_coordinate;
out vec3 frag_color;

void main()
{
    frag_position = v_position;
    frag_normal = v_normal;
    frag_coordinate = v_coordinate;
    frag_color = v_color;

    gl_Position = Projection * View * Model * vec4(v_position, 1.0);
    vec4 tmp = player_screen_pos - gl_Position;
    gl_Position[1] = gl_Position[1] - pow(length(tmp), 2) * 0.0075;

}
