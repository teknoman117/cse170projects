#version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;

uniform mat4 vTransf;
uniform mat4 vProj;

out vec3 Norm;
out vec4 Pos;

void main ()
{
    Norm = normalize(vNorm * mat3(vTransf));
    
    Pos = vec4(vPos,1.0) * vTransf;
    
    gl_Position = Pos * vProj;
}
