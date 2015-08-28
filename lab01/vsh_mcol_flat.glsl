#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec4 vColor;
uniform mat4 vTransf;
uniform mat4 vProj;
out vec4 Position;

void main ()
{
    Position = vec4 ( vPos.x, vPos.y, vPos.z, 1.0 );
    gl_Position = vProj * vTransf * Position;
}
