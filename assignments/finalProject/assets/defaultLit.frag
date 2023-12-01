#version 450
out vec4 FragColor;

in Surface{
	vec2 UV; //Per-fragment interpolated UV
	vec3 WorldPosition; //Per-fragment interpolated world position
	vec3 WorldNormal; //Per-fragment interpolated world normal
}fs_in;

/* 
UPDATED LIGHT STRUCT TO SUPPORT THE FOLLOWING - JERRY KAUFMAN
	- LIGHT TYPES
		- 0 : POINT LIGHT
 		- 1 : DIRECTIONAL 
		- 2 : SPOTLIGHT
	- RADIUS : RADIUS OF LIGHT
	- PENUMBRA : SPOTLIGHT CUTOFF ANGLE
	- UMBRA : SPOTLIGHT OUTER CUTOFF ANGLE
*/

struct Light
{	int lightType;
	vec3 position, color;
	float radius, penumbra, umbra;
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

	for(int i = 0; i < _NumLights; i++) {
		vec3 direction;
		float lightDistance;
		float lightIntensity;

		if (_Lights[i].lightType == 0) { // POINT LIGHT
			direction = normalize(_Lights[i].position - fs_in.WorldPosition);
			lightDistance = length(_Lights[i].position - fs_in.WorldPosition);

			lightIntensity = pow(clamp((1.0 - pow((lightDistance / _Lights[i].radius), 4)), 0, 1), 2);
		} else if (_Lights[i].lightType == 1) { // DIRECTIONAL
			direction = normalize(-_Lights[i].position);
		} else if (_Lights[i].lightType == 2) { // SPOTLIGHT
		} else if (_Lights[i].lightType == -1) { // NONE
		}


		//Ambient
		lightColor += _Material.ambientK * _Lights[i].color;

		//Diffuse
		vec3 w = normalize(_Lights[i].position - fs_in.WorldPosition);
		lightColor += _Lights[i].color * _Material.diffuseK * max(dot(normal, w), 0);

		//Specular 
		vec3 h = normalize(w + v);
		lightColor += _Lights[i].color * _Material.specular * pow(max(dot(h,normal),0),_Material.shininess);

		// TODO FIGURE OUT WHY EVERYTHING IS GETTING NO TEXTURE
		//lightColor *= lightIntensity;
	}
	
	FragColor = vec4(newTexture.rgb * lightColor, 1.0f);
}