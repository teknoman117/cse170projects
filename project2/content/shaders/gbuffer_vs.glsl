#version 410

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

smooth out vec3 Position;
smooth out vec3 Normal;

uniform mat4 M;
uniform mat4 VP;

void main ()
{
    vec4 p = M * vec4(vPosition, 1.0);
    vec4 n = M * vec4(vNormal, 0.0);

    Position = p.xyz;
    Normal   = n.xyz;

    gl_Position = VP * p;
}