#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTex;

out vec4 ourColor;
out float draw;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 mvp;

uniform sampler2D trimm_texture;

void main()
{
    draw = 1.0f;
    vec4 trimming = texture(trimm_texture, aTex);
	if(trimming.x > 0.5) draw = 0.0f;
    gl_Position = mvp * vec4(aPos, 1.0);
    ourColor = aColor;
}