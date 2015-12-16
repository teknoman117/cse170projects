#version 420
#extension GL_ARB_enhanced_layouts : enable

layout(vertices=1) out;

layout (std140, binding=0) uniform CameraParameters
{
    mat4 V;
    mat4 P;
    mat4 VP;

    vec3 CameraPosition;
    vec4 frustumPlanes[6];
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

layout (location = 1) in block 
{
    float minHeight;
    float maxHeight;
} In[];

layout(location=1) out block 
{
    vec2 tessLevelInner;
} Out[];

vec3 GetVertexAtGridLocation(vec2 location, ivec2 gridSize)
{
    vec2 blend = location / vec2(gridSize - 1);
    vec3 p     = mix(mix(nw, ne, blend.x), mix(sw, se, blend.x), blend.y);
    return p;
}

// Get the "positive" vertex based on an aabb and normal
vec3 AABBGetVertexP(vec3 a, vec3 b, vec3 n)
{
    vec3 P = a;

    if(n.x >= 0)
        P.x = b.x;
    if(n.y >= 0)
        P.y = b.y;
    if(n.z >= 0)
        P.z = b.z;

    return P;
}

// Get the "negative" vertex based on an aabb and normal
vec3 AABBGetVertexN(vec3 a, vec3 b, vec3 n)
{
    vec3 P = b;

    if(n.x >= 0)
        P.x = a.x;
    if(n.y >= 0)
        P.y = a.y;
    if(n.z >= 0)
        P.z = a.z;

    return P;
}

// Checks if an AABB is inside a frustum (intersects OR inside returns true)
bool AABBInFrustum(vec3 a, vec3 b, vec4 plane[6])
{
    for(int i = 0; i < 6; i++)
    {
        vec4 p = vec4(AABBGetVertexP(a,b,plane[i].xyz), 1);
        if(dot(p, plane[i]) < 0)
            return false;
    }

    return true;
}

void main ()
{
    // Pass the grid coordinates along
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
 
    // Compute the bounding box
    vec2 l1 = gl_in[gl_InvocationID].gl_Position.xz;
    vec2 l2 = gl_in[gl_InvocationID].gl_Position.xz + vec2(chunkSize);
    vec3 a_ = GetVertexAtGridLocation(l1, dataSize);
    vec3 b_ = GetVertexAtGridLocation(l2, dataSize);
    vec3 a = min(a_,b_);
    vec3 b = max(a_,b_);
    a.y = In[gl_InvocationID].minHeight;
    b.y = In[gl_InvocationID].maxHeight;

    // Visibility check
    //bool visible = AABBInFrustum(a, b, frustumPlanes);
    //if(visible)
    //{
        // force tess for the moment
        gl_TessLevelOuter[0] = 1;
        gl_TessLevelOuter[1] = 1;
        gl_TessLevelOuter[2] = 1;
        gl_TessLevelOuter[3] = 1;

        gl_TessLevelInner[0] = 1;
        gl_TessLevelInner[1] = 1;

        Out[gl_InvocationID].tessLevelInner = vec2(gl_TessLevelInner[0], gl_TessLevelInner[1]);
    /*}
    else
    {
        // cull patch
        gl_TessLevelOuter[0] = -1;
        gl_TessLevelOuter[1] = -1;
        gl_TessLevelOuter[2] = -1;
        gl_TessLevelOuter[3] = -1;

        gl_TessLevelInner[0] = -1;
        gl_TessLevelInner[1] = -1;
    }*/
}