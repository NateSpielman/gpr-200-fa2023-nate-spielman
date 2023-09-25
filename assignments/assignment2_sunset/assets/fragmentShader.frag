#version 450
	out vec4 FragColor;
	in vec2 UV;

	//Resolution
	uniform vec2 _Resolution;

	//Background Colors
	uniform vec3 _DayBottom;
	uniform vec3 _DayTop;
	uniform vec3 _NightBottom;
	uniform vec3 _NightTop;

	//Time
	uniform float _Time;
	uniform float _TimeSpeed;

	//Sun Radius
	uniform float _SunR;

	//Sun Color
	uniform vec3 _SunColor;

	//Mountains Color
	uniform vec3 _MountainColor;

	//Hills Color
	uniform vec3 _HillsColor;

	void main(){
		
		//Setting coordinates and aspect ratio
		vec2 UV = UV * 2.0 - 1.0;
		float aspectRatio = _Resolution.x / _Resolution.y;
		UV.x *= aspectRatio;

		//Background Gradients
		vec3 bgDay = mix(_DayBottom, _DayTop, UV.y + 0.25);
		vec3 bgNight = mix(_NightBottom, _NightTop, UV.y + 0.25);

		//Set Time
		float t = sin(_Time * _TimeSpeed) * 0.5 + 0.5;
    
		//Draw Background
		vec3 col = mix(bgNight, bgDay, t);

		//Sun
		float sun = distance(UV, vec2(0.0, sin(_Time * _TimeSpeed) - 0.5));
		sun = smoothstep(_SunR, _SunR + 0.2, sun);

		//Draw Sun
		col = mix(_SunColor, col, sun); 

		//Mountains time
		float tX = UV.x + _Time * 0.5;

		//Mountains Shape
		float mountains = 1.0 - step(abs(sin(tX * 4.5) + sin(tX + 0.8)) * 0.2 - 0.2, UV.y);

		//Draw Mountains
		col = mix(col, _MountainColor, mountains);

		//Hills time
		tX = UV.x + _Time;

		//Hills Shape
		float hills = 1.0 - step((sin(tX * 0.8) + sin(tX * 2.0)) * 0.15 - 0.6, UV.y);

		//Draw Hills
		col = mix(col, _HillsColor, hills);

		// Output to screen
		FragColor = vec4(col,1.0);
	}