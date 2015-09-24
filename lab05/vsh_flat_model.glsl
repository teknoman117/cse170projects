# version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (location = 2) in vec4 vColor;

uniform mat4 vTransf;
uniform mat4 vProj;

flat out vec3 Norm;
out vec4 Pos;
out vec4 Color;

void main ()
{
    Norm = normalize(vNorm * mat3(vTransf));
    Pos = vec4(vPos, 1.0) * vTransf;
    Color = vColor / 255.0;
    
    gl_Position = Pos * vProj;

    // Note: our vector appears on the left side of the above multiplication
    // because mat4 stores the received values in column-major format, what causes
    // the effect of transposing our original values.
}
