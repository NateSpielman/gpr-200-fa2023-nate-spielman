#version 450
	out vec4 FragColor;
	uniform vec3 _Color;
	uniform float _Brightness;
	void main(){
		FragColor = vec4(_Color * _Brightness,1.0);
	}