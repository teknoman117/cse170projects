# version 400

layout (location=0) in vec3 vPos;
layout (location=1) in vec3 vNorm;
layout (location=2) in vec2 vTexCoord;

out vec3 Pos;
out vec3 Norm;
out vec2 TexCoord;

uniform mat4 vTransf;
uniform mat4 vProj;

void main ()
{
    vec4 p = vec4(vPos,1.0)*vTransf; // vertex pos in eye coords
    gl_Position = p * vProj;

    Pos = vec3(p);
    Norm = normalize ( vNorm*mat3(vTransf) ); // vertex normal
    TexCoord = vTexCoord;
}
