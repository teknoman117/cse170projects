#version 410

layout(location = 0) in vec2 position;

smooth out vec2 textureCoordinate;

void main()
{
	textureCoordinate = (position + vec2(1,1)) / 2; 
	gl_Position = vec4(position, 0, 1);
}
