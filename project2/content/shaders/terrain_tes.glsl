#version 420
#extension GL_ARB_enhanced_layouts : enable

layout(quads, equal_spacing, cw) in;

uniform sampler2D heightmap;

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

layout(location=1) in block 
{
    vec2 tessLevelInner;
} In[];

layout(location = 1) out block 
{
    vec3 position;
    vec3 normal;
} Out;

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}

// borrowed from https://github.com/ashima/webgl-noise/blob/master/src/noise2D.glsl
float snoise(vec2 v)
{
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                       -0.577350269189626,  // -1.0 + 2.0 * C.x
                        0.024390243902439); // 1.0 / 41.0
    // First corner
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);

    // Other corners
    vec2 i1;
    //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
    //i1.y = 1.0 - i1.x;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    // x0 = x0 - 0.0 + 0.0 * C.xx ;
    // x1 = x0 - i1 + 1.0 * C.xx ;
    // x2 = x0 - 1.0 + 2.0 * C.xx ;
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    // Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
        + i.x + vec3(0.0, i1.x, 1.0 ));

    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;

    // Gradients: 41 points uniformly over a line, mapped onto a diamond.
    // The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt( a0*a0 + h*h );
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

    // Compute final noise value at P
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

vec3 GetVertexAtGridLocation(vec2 location, ivec2 areaSize, vec2 pixelSize)
{
    vec2 blend = location / vec2(areaSize - 1);
    vec3 p     = mix(mix(nw, ne, blend.x), mix(sw, se, blend.x), blend.y);
    vec2 uv    = (location * pixelSize) + (pixelSize / 2.0);
    
    p.y = texture(heightmap, uv).r;
    return p;
}

float fbm(vec2 p)
{
    float total = 0.0;
    float octaves = 9;
    float lacunarity = 2.0;
    float gain = 1 / lacunarity;
    float frequency = 1.0/16.0;
    float amplitude = 2*gain*gain; 

    for(int i = 0; i < octaves; i++)
    {
        total += snoise(p * vec2(frequency,frequency)) * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }

    return total;
}

void main ()
{
    // Compute some constants
    vec2 chunkSizef  = vec2(chunkSize);
    vec2 location    = gl_in[0].gl_Position.xz + gl_TessCoord.xy * chunkSizef;
    vec2 pixelSize   = 1.0 / vec2(rasterSize);

    // Compute the surrounding points (based on tessellation level)
    vec2 pSize = chunkSizef / In[0].tessLevelInner;

    vec3 L11   = GetVertexAtGridLocation(location+vec2(0, 0),        dataSize, pixelSize);
    vec3 L21   = GetVertexAtGridLocation(location+vec2(0, pSize.y),  dataSize, pixelSize);
    vec3 L01   = GetVertexAtGridLocation(location+vec2(0, -pSize.y), dataSize, pixelSize);
    vec3 L12   = GetVertexAtGridLocation(location+vec2(pSize.x, 0),  dataSize, pixelSize);
    vec3 L10   = GetVertexAtGridLocation(location+vec2(-pSize.x, 0), dataSize, pixelSize);

    L21.y += fbm(L21.xz);
    L01.y += fbm(L01.xz);
    L12.y += fbm(L12.xz);
    L10.y += fbm(L10.xz);

    // Compute the normal
    vec3 dx = L12 - L10;
    vec3 dz = L01 - L21;
    vec3 N = normalize(cross(dx,dz));

    L11 = L11 + N*fbm(L11.xz);

    // Compute output
    Out.position = L11;
    Out.normal   = N;
    gl_Position  = VP * vec4(L11, 1);
}
