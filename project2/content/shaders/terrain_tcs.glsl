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

uniform bool tessellationEnabled;

layout (location = 1) in block 
{
    float minHeight;
    float maxHeight;
} In[];

layout(location=1) out block 
{
    vec2 tessLevelInner;
} Out[];

vec3 GetVertexAtGridLocation(vec2 location, ivec2 areaSize)
{
    vec2 blend = location / vec2(areaSize - 1);
    vec3 p     = mix(mix(nw, ne, blend.x), mix(sw, se, blend.x), blend.y);
    return p;
}

bool TestSphereInFrustum(vec3 pos, float r, vec4 plane[6])
{
    for(int i=0; i<6; i++) 
    {
        if (dot(vec4(pos, 1.0), plane[i]) + r < 0.0) 
        {
            return false;
        }
    }
    return true;
}

// transform from world to screen coordinates
vec2 worldToScreen(vec3 p)
{
    vec4 r = VP * vec4(p, 1.0);   // to clip space
    r.xy /= r.w;            // project
    r.xy = r.xy*0.5 + 0.5;  // to NDC
    r.xy *= viewport.zw;    // to pixels
    return r.xy;
}

// calculate edge tessellation level from two edge vertices in screen space
float calcEdgeTessellation(vec2 s0, vec2 s1, float s)
{
    float d = distance(s0, s1);
    return clamp(d / s, 1, 64);
}

vec2 eyeToScreen(vec4 p)
{
    vec4 r = P * p;   // to clip space
    r.xy /= r.w;            // project
    r.xy = r.xy*0.5 + 0.5;  // to NDC
    r.xy *= viewport.zw;    // to pixels
    return r.xy;
}

// calculate tessellation level by fitting sphere to edge
float calcEdgeTessellationSphere(vec3 w0, vec3 w1, float diameter, float s)
{
    vec3 centre = (w0 + w1) * 0.5;
    vec4 view0 = V * vec4(centre, 1.0);
    vec4 view1 = view0 + vec4(diameter, 0, 0, 0);
    vec2 s0 = eyeToScreen(view0);
    vec2 s1 = eyeToScreen(view1);
    return calcEdgeTessellation(s0, s1, s);
}

void main ()
{
    // Pass the grid coordinates along
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
 
    // Compute the indicies of the chunk diagonals borders
    vec2 i0 = gl_in[gl_InvocationID].gl_Position.xz;
    vec2 i2 = i0 + vec2(chunkSize);
    vec3 v0 = GetVertexAtGridLocation(i0, dataSize);
    vec3 v2 = GetVertexAtGridLocation(i2, dataSize);

    // Compute the bounding sphere of the tile
    v0.y = In[gl_InvocationID].minHeight;
    v2.y = In[gl_InvocationID].maxHeight;
    vec3  p = (v2+v0) * 0.5;
    float r = length(v2-v0) * 0.5;

    // Visibility check
    bool visible = TestSphereInFrustum(p, r, frustumPlanes);
    if(visible)
    {
        // Look up other tile points
        float averageHeight = (In[gl_InvocationID].minHeight + In[gl_InvocationID].maxHeight) * 0.5;

        vec2 i1 = i0 + vec2(chunkSize.x, 0);
        vec2 i3 = i0 + vec2(0, chunkSize.y);
        vec3 v1 = GetVertexAtGridLocation(i1, dataSize);
        vec3 v3 = GetVertexAtGridLocation(i3, dataSize);

        v0.y = averageHeight;
        v1.y = averageHeight;
        v2.y = averageHeight;
        v3.y = averageHeight;

        if(tessellationEnabled)
        {
            float sphereD = chunkSize.x*2;
            gl_TessLevelOuter[0] = calcEdgeTessellationSphere(v3, v0, sphereD, 2);
            gl_TessLevelOuter[1] = calcEdgeTessellationSphere(v0, v1, sphereD, 2);
            gl_TessLevelOuter[2] = calcEdgeTessellationSphere(v1, v2, sphereD, 2);
            gl_TessLevelOuter[3] = calcEdgeTessellationSphere(v2, v3, sphereD, 2);
        } 
        else
        {
            gl_TessLevelOuter[0] = 1;
            gl_TessLevelOuter[1] = 1;
            gl_TessLevelOuter[2] = 1;
            gl_TessLevelOuter[3] = 1;
        }

        gl_TessLevelInner[0] = 0.5 * (gl_TessLevelOuter[1] + gl_TessLevelOuter[3]);
        gl_TessLevelInner[1] = 0.5 * (gl_TessLevelOuter[0] + gl_TessLevelOuter[2]);

        Out[gl_InvocationID].tessLevelInner = vec2(gl_TessLevelInner[0], gl_TessLevelInner[1]);
    }
    else
    {
        // cull patch
        gl_TessLevelOuter[0] = -1;
        gl_TessLevelOuter[1] = -1;
        gl_TessLevelOuter[2] = -1;
        gl_TessLevelOuter[3] = -1;

        gl_TessLevelInner[0] = -1;
        gl_TessLevelInner[1] = -1;
    }
}
