#version 330
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 _Projection;
uniform mat4 _View;

void main()
{
    vec4 pos = _Projection * _View * vec4(aPos, 1.0f);
    // Having z equal w will always result in a depth of 1.0f
    gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
    // We want to flip the z axis due to the different coordinate systems (left hand vs right hand)
    TexCoords = vec3(aPos.x, aPos.y, -aPos.z);
}  