#version 450
out vec4 FragColor;

in Surface{
	vec2 UV; //Per-fragment interpolated UV
	vec3 WorldPosition; //Per-fragment interpolated world position
	vec3 WorldNormal; //Per-fragment interpolated world normal
}fs_in;

uniform sampler2D _Texture;

struct Light
{
	vec3 position;
	vec3 color;
};
#define MAX_LIGHTS 1
uniform Light _Lights[MAX_LIGHTS];

void main(){
	FragColor = texture(_Texture,fs_in.UV);
	vec3 normal = normalize(fs_in.WorldNormal);
	//TODO: Lighting calculations using corrected normal
	vec3 color = _Lights[0].color; 
}