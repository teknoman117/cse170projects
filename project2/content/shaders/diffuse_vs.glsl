#version 410

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

smooth out vec3 Position;
smooth out vec3 Normal;

uniform mat4 MV;
uniform mat4 P;

void main ()
{
    vec4 p = MV * vec4(vPosition, 1.0);
    vec4 n = MV * vec4(vNormal, 0.0);

    Position = p.xyz;
    Normal   = n.xyz;

    gl_Position = P * p;
}