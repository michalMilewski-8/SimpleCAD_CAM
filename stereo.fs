#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTextureL;
uniform sampler2D screenTextureR;

void main()
{ 
    vec4 FragColorL = texture(screenTextureL, TexCoords);
    vec4 FragColorR = texture(screenTextureR, TexCoords);

    FragColor = FragColorL;
    FragColor.x = FragColorR.x;

}