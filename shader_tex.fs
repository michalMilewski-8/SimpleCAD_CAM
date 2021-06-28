#version 330 core
out vec4 FragColor;

in vec4 ourColor;
in float draw;

void main()
{
    if(draw < 0.5f) discard;
    FragColor = ourColor;
}