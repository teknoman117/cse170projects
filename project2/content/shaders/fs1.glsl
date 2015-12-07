#version 410

smooth in vec4 theColor;
smooth in vec3 thePosition;

layout (location = 0) out vec4 gBufferDiffuseSpecular;
layout (location = 1) out vec3 gBufferNormals;
layout (location = 2) out vec3 gBufferPosition;

void main()
{
    gBufferDiffuseSpecular = vec4(theColor.rgb, 1);
    gBufferNormals = vec3(0,0,1);
    gBufferPosition = thePosition;
}
