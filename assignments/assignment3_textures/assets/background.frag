#version 450
out vec4 FragColor;
in vec2 UV;

//Time
uniform float _Time;
uniform float _TimeSpeed;

//Textures
uniform sampler2D _Texture;
uniform sampler2D _NoiseTexture;

void main(){
	//Set Time
	float t = sin(_Time * _TimeSpeed) * 0.5 + 0.5;

	float noise = texture(_NoiseTexture, UV).r;	
	vec2 uv = UV + noise;
	vec4 color = texture(_Texture, uv);
	
	FragColor = vec4(color);
}