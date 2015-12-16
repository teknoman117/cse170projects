#version 420
#extension GL_ARB_enhanced_layouts : enable

layout(quads, fractional_even_spacing, cw) in;

uniform sampler2D heightmap;

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

layout(location=1) in block 
{
    vec2 tessLevelInner;
} In[];

layout(location = 1) out block 
{
    vec3 position;
    vec3 normal;
} Out;

vec3 GetVertexAtGridLocation(vec2 location, ivec2 areaSize, vec2 pixelSize)
{
    vec2 blend = location / vec2(areaSize - 1);
    vec3 p     = mix(mix(nw, ne, blend.x), mix(sw, se, blend.x), blend.y);
    vec2 uv    = (location * pixelSize) + (pixelSize / 2.0);
    
    // TODO - fractals
    p.y = texture(heightmap, uv).r;

    return p;
}

void main ()
{
    // Compute some constants
    vec2 chunkSizef  = vec2(chunkSize);
    vec2 location    = gl_in[0].gl_Position.xz + gl_TessCoord.xy * chunkSizef;
    vec2 pixelSize   = 1.0 / vec2(rasterSize);

    // Compute the surrounding points (based on tessellation level)
    vec2 pSize = chunkSizef / In[0].tessLevelInner;
    vec3 L11   = GetVertexAtGridLocation(location+vec2(0, 0),       dataSize, pixelSize);
    vec3 L21   = GetVertexAtGridLocation(location+vec2(0, pSize.y), dataSize, pixelSize);
    vec3 L12   = GetVertexAtGridLocation(location+vec2(pSize.x, 0), dataSize, pixelSize);

    // Compute the normal
    vec3 dx = L12 - L11;
    vec3 dz = L11 - L21;
    vec3 N = normalize(cross(dx,dz));

    // Compute output
    Out.position = L11;
    Out.normal   = N;
    gl_Position  = VP * vec4(L11, 1);
}
