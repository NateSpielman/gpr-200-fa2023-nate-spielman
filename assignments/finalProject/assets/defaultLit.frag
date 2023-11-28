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

#define MAX_LIGHTS 4
uniform Light _Lights[MAX_LIGHTS];
uniform int _NumLights;
uniform Material _Material;
uniform vec3 _CamPos;
uniform sampler2D _Texture;

void main(){
	vec3 normal = normalize(fs_in.WorldNormal);
	vec4 newTexture =  texture(_Texture,fs_in.UV);
	vec3 v = normalize(_CamPos - fs_in.WorldPosition);

	vec3 lightColor;

	for(int i =0; i < _NumLights; i++) 
	{
		//Ambient
		lightColor += _Material.ambientK * _Lights[i].color;

		//Diffuse
		vec3 w = normalize(_Lights[i].position - fs_in.WorldPosition);
		lightColor += _Lights[i].color * _Material.diffuseK * max(dot(normal, w), 0);

		//Specular 
		vec3 h = normalize(w + v);
		lightColor += _Lights[i].color * _Material.specular * pow(max(dot(h,normal),0),_Material.shininess);
	}
	
	FragColor = vec4(newTexture.rgb * lightColor, 1.0f);
}