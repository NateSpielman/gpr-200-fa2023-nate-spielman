#version 450
out vec4 FragColor;
in vec2 UV;

//Time
uniform float _Time;
uniform float _TimeSpeed;

//Texture
uniform sampler2D _Texture;

void main(){ 
	vec2 UV = UV * 2.0 - 0.5;

	//Set Time
	float t = sin(_Time * _TimeSpeed) * 0.5 + 0.5;

	//move character 

	
	FragColor = texture(_Texture, UV);
}