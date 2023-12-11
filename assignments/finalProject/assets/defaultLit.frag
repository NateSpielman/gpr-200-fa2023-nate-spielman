#version 450
out vec4 FragColor;

in Surface{
	vec2 UV; 
	vec3 WorldPosition, WorldNormal; 
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
	- DIRECTION : AREA THAT CAN EMIT LIGHT
*/

struct Light {	
	int lightType;
	float radius, penumbra, umbra;
	vec3 position, direction, color;
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

/*     Pre:  Uniform values from Lights distance and radius. Takes in clamp range. 
*  Purpose:  Calculate UE windows for spotlight and point light
*************************************************************/
float calculateWindowed(float lDistance, float lRadius, int clampValue) {
	return pow(clamp((1.0 - pow((lDistance / lRadius), 4)), 0, 1), clampValue);
}

void main(){
	vec3 normal = normalize(fs_in.WorldNormal);
	vec4 newTexture =  texture(_Texture,fs_in.UV);
	vec3 v = normalize(_CamPos - fs_in.WorldPosition);
	vec3 totalLightColor, h;
	float lightIntensity = 1.0f;

	for(int i = 0; i < _NumLights; i++) {
		vec3 lightColor = vec3(0.0), w;
        float lightIntensity = 1.0;
        float attenuation = 1.0, lightDistance;

		switch (_Lights[i].lightType) {
			 // POINT LIGHT
			 case 0: 
				w = normalize(_Lights[i].position - fs_in.WorldPosition);
				lightDistance = length(_Lights[i].position - fs_in.WorldPosition);

				// CALCULATES UE WINDOWED USING THE LIGHT DISTANCE AND THE RADIUS OF THE LIGHTS WITH CLAMPED 0-1
				lightIntensity = calculateWindowed(lightDistance, _Lights[i].radius, 2);
				break;
			// DIRECTIONAL
			case 1:
				w = normalize(-_Lights[i].direction); 
				break;
			// SPOTLIGHT
			case 2:
				w = normalize(_Lights[i].position - fs_in.WorldPosition);

                float cosTheta = dot(-w, normalize(_Lights[i].direction)); 
                float intensityFactor = smoothstep(_Lights[i].umbra, _Lights[i].penumbra, cosTheta);

				lightDistance = length(_Lights[i].position - fs_in.WorldPosition);

				// CALCULATES UE WINDOWED USING THE LIGHT DISTANCE AND THE RADIUS OF THE LIGHTS WITH CLAMPED 0-1
				attenuation = calculateWindowed(lightDistance, _Lights[i].radius, 2); 

				lightIntensity *= (intensityFactor * attenuation);

				break;
			// NONE
			default:
				// NO EMISSION OF LIGHT
				totalLightColor += vec3(0.0);
				break;
		}

		if (_Lights[i].lightType != -1) {
				//Ambient
				lightColor += _Material.ambientK * _Lights[i].color;

				//Diffuse
				lightColor += _Lights[i].color * _Material.diffuseK * max(dot(normal, w), 0);

				//Specular 
				h = normalize(w + v);
				lightColor += _Lights[i].color * _Material.specular * pow(max(dot(h,normal),0),_Material.shininess);

				lightColor *= lightIntensity;

				totalLightColor += lightColor;
		}
	}
	
	FragColor = vec4(newTexture.rgb *= totalLightColor, 1.0);
}