#version 410

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

uniform float amplification;

smooth out vec4 theColor;

void main()
{
	gl_Position = vec4(position, 1);
	theColor = color * amplification;
}
