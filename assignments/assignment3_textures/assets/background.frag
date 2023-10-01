#version 450
out vec4 FragColor;
in vec2 UV;

//Time
uniform float _Time;
uniform float _TimeSpeed;

//Distortion Intensity
uniform float _Distortion;

//Textures
uniform sampler2D _Texture;
uniform sampler2D _NoiseTexture;

void main(){
	//Noise coordinates that offset from the UV so they scroll over time
	vec2 noiseCoord = vec2(UV.x + (_Time * _TimeSpeed), UV.y);

	//Sample from noise texture
	float noise = texture(_NoiseTexture, noiseCoord).r;	

	//Offset UV coordinates for background texture
	//Minuses by _Distortion/2 to recenter the background
	vec2 uv = UV + (noise * _Distortion) - (_Distortion / 2);
	
	FragColor = texture(_Texture, uv);
}