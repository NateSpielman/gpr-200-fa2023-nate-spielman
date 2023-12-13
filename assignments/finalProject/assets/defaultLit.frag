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

//Terrain texture
uniform sampler2D _TextureSnow;
uniform sampler2D _TextureGrass;
uniform sampler2D _TextureRock;

//Terrain uniforms
uniform float _terMinY;
uniform float _terMaxY;

uniform float _HBTrange1;
uniform float _HBTrange2;
uniform float _HBTrange3;
uniform float _HBTrange4;

/*     Pre:  Uniform values from Lights distance and radius. Takes in clamp range. 
*  Purpose:  Calculate UE windows for spotlight and point light
*************************************************************/
float calculateWindowed(float lDistance, float lRadius, int clampValue) {
	return pow(clamp((1.0 - pow((lDistance / lRadius), 4)), 0, 1), clampValue);
}

/*     Pre:  Scale of where vertex is vertically relation to the rest of terrain. 
 *  Purpose:  Calculate texture based on height of vertex
*************************************************************/
vec4 heightBasedTexture(float scaleIn)
{
		vec4 color;

		//If vertex below range 1 texture as rock
		if (scaleIn >= 0.0 && scaleIn <= _HBTrange1)
		{
			color = texture(_TextureRock,fs_in.UV);
		}
		//If vertex between range1 and range2 texture as mix between rock and grass
		else if (scaleIn <= _HBTrange2)
		{
			scaleIn -= _HBTrange1;
			scaleIn /= (_HBTrange2 - _HBTrange1);

			float scaletmp = scaleIn;
			scaleIn = 1.0f - scaleIn;

			color += texture(_TextureRock,fs_in.UV) * scaleIn;
			color += texture(_TextureGrass,fs_in.UV) * scaletmp;
		}
		//If vertex between range2 and range 3 texture as grass
		else if (scaleIn <= _HBTrange3)
		{
			color = texture(_TextureGrass,fs_in.UV);
		}
		//If vertex between range3 and range4 texture as mix between grass and snow
		else if (scaleIn <= _HBTrange4)
		{
			scaleIn -= _HBTrange3;
			scaleIn /= (_HBTrange4 - _HBTrange3);

			float scaletmp = scaleIn;
			scaleIn = 1.0f - scaleIn;

			color += texture(_TextureGrass,fs_in.UV) * scaleIn;
			color += texture(_TextureSnow,fs_in.UV) * scaletmp;
		}
		//If vertex above range4 texture as snow
		else
		{
			color = texture(_TextureSnow,fs_in.UV);
		}

		return color;
}

void main(){
	vec3 normal = normalize(fs_in.WorldNormal);

	float scale = abs(fs_in.WorldPosition.y - _terMinY) / abs(_terMaxY - _terMinY);
	vec4 newTexture =  heightBasedTexture(scale);

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
				attenuation = calculateWindowed(lightDistance, _Lights[i].radius, 2);
				
				lightIntensity *= attenuation;
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
