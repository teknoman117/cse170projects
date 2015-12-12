#version 410

//layout (location = 0) in vec3 vPosition;
//layout (location = 1) in vec3 vNormal;
layout (location = 0) in uint dummy;

uniform sampler2D heightmap;

smooth out vec3 Position;
smooth out vec3 Normal;

uniform mat4 M;
uniform mat4 VP;

uniform vec3 ne;
uniform vec3 nw;
uniform vec3 se;
uniform vec3 sw;

uniform ivec2 dataSize;

vec3 GetVertexAtGridLocation(vec2 location, ivec2 gridSize, vec2 pixelSize)
{
    vec2 blend = location / vec2(gridSize - 1);
    vec3 p     = mix(mix(nw, ne, blend.x), mix(sw, se, blend.x), blend.y);
    vec2 uv    = (location * pixelSize) + (pixelSize / 2.0);
    
    // TODO - fractals
    p.y = texture(heightmap, uv).r;

    return p;
}

void main ()
{
    // Compute some constant data
    vec2 location   = vec2(gl_VertexID % dataSize.x, gl_VertexID / dataSize.x);
    vec2 rasterSize = vec2(textureSize(heightmap,0));
    vec2 pixelSize  = 1.0 / rasterSize;

    vec3 L11 = GetVertexAtGridLocation(location+vec2( 0, 0), dataSize, pixelSize);
    vec3 L01 = GetVertexAtGridLocation(location+vec2( 0,-1), dataSize, pixelSize);
    vec3 L21 = GetVertexAtGridLocation(location+vec2( 0, 1), dataSize, pixelSize);
    vec3 L10 = GetVertexAtGridLocation(location+vec2(-1, 0), dataSize, pixelSize);
    vec3 L12 = GetVertexAtGridLocation(location+vec2( 1, 0), dataSize, pixelSize);

    vec3 dx = normalize(L12-L10);
    vec3 dy = normalize(L01-L21);

    vec4 n = M * vec4(cross(dx,dy), 1);
    vec4 p = M * vec4(L11,          1);

    Position = p.xyz;
    Normal   = n.xyz; 

    gl_Position = VP * p;
}