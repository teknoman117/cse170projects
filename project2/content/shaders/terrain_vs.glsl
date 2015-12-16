#version 420
#extension GL_ARB_enhanced_layouts : enable

layout (std140, binding=0) uniform CameraParameters
{
    mat4 V;
    mat4 P;
    mat4 VP;

    vec3 CameraPosition;
    
    vec4 frustumPlanes[6];
    vec4 viewport;
};

layout (std140, binding=1) uniform TerrainParameters
{
    ivec2 rasterSize;
    ivec2 dataSize;
    ivec2 gridSize;
    ivec2 chunkSize;

    vec3 ne;
    vec3 nw;
    vec3 se;
    vec3 sw;

    float triSize;
};

layout (location = 0) in vec2 height;

layout (location = 1) out block 
{
    float minHeight;
    float maxHeight;
} Out;

// Pass through the grid coordinates of the vertex and the maximum height
void main ()
{
    Out.minHeight = height.x;
    Out.maxHeight = height.y;
    gl_Position = vec4((gl_InstanceID % gridSize.x) * chunkSize.x, 0, (gl_InstanceID / gridSize.x) * chunkSize.y, 1);
}
