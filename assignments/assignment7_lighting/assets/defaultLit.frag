#version 450
out vec4 FragColor;

in Surface{
	vec2 UV; //Per-fragment interpolated UV
	vec3 WorldPosition; //Per-fragment interpolated world position
	vec3 WorldNormal; //Per-fragment interpolated world normal
}fs_in;

struct Light
{
	vec3 position;
	vec3 color;
};

struct Material {
	float ambientK; 
	float diffuseK; 
	float specular; 
	float shininess;
};

//#define MAX_LIGHTS 1
//uniform Light _Lights[MAX_LIGHTS];
uniform Light _Light;
uniform Material _Material;
uniform sampler2D _Texture;

void main(){
	vec3 normal = normalize(fs_in.WorldNormal);
	
	//Diffuse
	vec3 w = normalize(_Light.position - fs_in.WorldPosition);
	vec3 color = _Light.color * (_Material.diffuseK * max(dot(normal, w), 0));
	
	FragColor = vec4(color, 1.0) * texture(_Texture,fs_in.UV);
}