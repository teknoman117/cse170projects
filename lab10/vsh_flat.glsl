#version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;

uniform mat4 vTransf;
uniform mat4 vProj;

uniform vec4 kd;

flat out vec4 Color;

void main ()
{
    vec4 p = vec4(vPos,1.0) * vTransf; // vertex pos in eye coords
    Color = kd;
    gl_Position = p * vProj;
}
