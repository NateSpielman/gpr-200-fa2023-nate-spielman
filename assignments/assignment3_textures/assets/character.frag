#version 450
out vec4 FragColor;
in vec2 UV;
in vec2 charaCoords;

//Texture
uniform sampler2D _Texture;

void main(){ 
	FragColor = texture(_Texture, charaCoords);
}