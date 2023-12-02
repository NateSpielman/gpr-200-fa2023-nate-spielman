#version 330
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube _Skybox;

void main()
{    
    FragColor = texture(_Skybox, TexCoords);
}