#version 410

smooth in vec4 theColor;

layout (location = 0) out vec4 gBufferDiffuseSpecular;
layout (location = 1) out vec4 gBufferNormals;
layout (location = 2) out vec4 gBufferPosition;

void main()
{
    gBufferDiffuseSpecular = vec4(theColor.rgb, 1);
}
