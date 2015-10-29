# version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec4 vColor;
layout (location = 2) in vec3 vNormal;

uniform mat4 vTransf;
uniform mat4 vProj;

out vec3 Pos;
out vec4 Color;
out vec3 Normal;

void main ()
{
    Color = vColor / 255.0;
    gl_Position = vec4 ( vPos, 1.0 ) * vTransf * vProj;

	Normal = vNormal;

	Pos = vPos;
 
    // Note: our vector appears on the left side of the above multiplication
    // because mat4 stores the received values in column-major format, what causes
    // the effect of transposing our original values.
}
