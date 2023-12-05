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
		- -1 : NONE
		-  0 : POINT LIGHT
 		-  1 : DIRECTIONAL 
		-  2 : SPOTLIGHT
	- RADIUS : RADIUS OF LIGHT
	- PENUMBRA : SPOTLIGHT CUTOFF ANGLE
	- UMBRA : SPOTLIGHT OUTER CUTOFF ANGLE
*/

struct Light {	
	int lightType;
	vec3 position, color, direction;
	float radius, penumbra, umbra;
};

struct Material {
	float ambientK, diffuseK; 
	float specular, shininess;
};

#define MAX_LIGHTS 4
uniform Light _Lights[MAX_LIGHTS];
uniform int _NumLights;
uniform Material _Material;
uniform vec3 _CamPos;
uniform sampler2D _Texture;

float calculateWindowed(float lDistance, float lRadius, int clampValue) {
	return pow(clamp((1.0 - pow((lDistance / lRadius), 4)), 0, 1), clampValue);
}

void main(){
	vec3 normal = normalize(fs_in.WorldNormal);
	vec4 newTexture =  texture(_Texture,fs_in.UV);
	vec3 v = normalize(_CamPos - fs_in.WorldPosition);
	vec3 totalLightColor;
	float lightIntensity = 1.0f;

	for(int i = 0; i < _NumLights; i++) {
		vec3 direction, lightColor;
		float lightDistance;

		switch (_Lights[i].lightType) {
			 // POINT LIGHT
			 case 0: 
				direction = normalize(_Lights[i].position - fs_in.WorldPosition);
				lightDistance = length(_Lights[i].position - fs_in.WorldPosition);

				lightIntensity = calculateWindowed(lightDistance, _Lights[i].radius, 2);
				break;
			// DIRECTIONAL
			case 1:
				direction = normalize(-_Lights[i].position);
				break;
			// SPOTLIGHT
			case 2:
				direction = normalize(_Lights[i].position - fs_in.WorldPosition);
				lightDistance = length(_Lights[i].position - fs_in.WorldPosition);

				float spotEffect, cutoff, outerCutoff, attenuation;

				

				break;
			// NONE
			default:
				break;
		}

		//Ambient
		lightColor += _Material.ambientK * _Lights[i].color;

		//Diffuse
		vec3 w = normalize(_Lights[i].position - fs_in.WorldPosition);
		lightColor += _Lights[i].color * _Material.diffuseK * max(dot(normal, w), 0);

		//Specular 
		vec3 h = normalize(w + v);
		lightColor += _Lights[i].color * _Material.specular * pow(max(dot(h,normal),0),_Material.shininess);

		lightColor *= lightIntensity;

		totalLightColor += lightColor;
	}
	
	FragColor = vec4(newTexture.rgb * totalLightColor, 1.0f);
}