#version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;

uniform mat4 vTransf;
uniform mat4 vProj;

out vec3 Norm;
out vec3 Pos;

void main ()
{
    Norm = (vec4(vNorm,0.0) * vTransf).xyz;
    Pos = (vec4(vPos,1.0) * vTransf).xyz;
    
    gl_Position = vec4(vPos,1.0) * vTransf * vProj;
}
