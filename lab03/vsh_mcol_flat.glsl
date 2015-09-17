# version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec4 vColor;

uniform mat4 vTransf;
uniform mat4 vProj;
uniform vec4 diffuse;

flat out vec4 Color;

void main ()
{
	Color = (vColor / 255.0) * diffuse;

	gl_Position = vec4 ( vPos, 1.0 ) * vTransf * vProj;

	// Note: our vector appears on the left side of the above multiplication
	// because mat4 stores the received values in column-major format, what causes
	// the effect of transposing our original values.
}
