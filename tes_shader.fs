#version 400 core
out vec4 FragColor;
in vec4 f_color;

void main()
{
    if(f_color.x < 0.5f) discard;
    FragColor = vec4(0.7f,0.7f,0.7f,0.5f);
}