#version 420
#extension GL_ARB_enhanced_layouts : enable

uniform sampler2D verticalTexture;
uniform sampler2D verticalTextureNormals;
uniform sampler2D horizontalTexture;
uniform sampler2D horizontalTextureNormals;

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

layout (location = 1) in block 
{
    vec3 position;
    vec3 normal;
} In;

layout (location = 0) out vec4 gBufferDiffuseSpecular;
layout (location = 1) out vec3 gBufferNormals;
layout (location = 2) out vec3 gBufferPosition;


mat3 inverse3x3( mat3 M )
{
    // The original was written in HLSL, but this is GLSL, 
    // therefore
    // - the array index selects columns, so M_t[0] is the 
    //   first row of M, etc.
    // - the mat3 constructor assembles columns, so 
    //   cross( M_t[1], M_t[2] ) becomes the first column
    //   of the adjugate, etc.
    // - for the determinant, it does not matter whether it is
    //   computed with M or with M_t; but using M_t makes it
    //   easier to follow the derivation in the text
    mat3 M_t = transpose( M ); 
    float det = dot( cross( M_t[0], M_t[1] ), M_t[2] );
    mat3 adjugate = mat3( cross( M_t[1], M_t[2] ),
                          cross( M_t[2], M_t[0] ),
                          cross( M_t[0], M_t[1] ) );
    return adjugate / det;
}

mat3 cotangent_frame( vec3 N, vec3 p, vec2 uv )
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
 
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}

vec3 perturb_normal( vec3 N, vec3 V, vec2 texcoord )
{
    // assume N, the interpolated vertex normal and 
    // V, the view vector (vertex to eye)
    vec3 map = texture2D( horizontalTextureNormals, texcoord ).xyz;
    map = map * 255./127. - 128./127.;
    map.y = -map.y;
    mat3 TBN = cotangent_frame( N, -V, texcoord );
    return normalize( TBN * map );
}

void main()
{
    vec3 blendWeights = abs(In.normal);
    //blendWeights = blendWeights * vec3(1.0,0.9,1.0);
    blendWeights = max(blendWeights, vec3(0,0,0));
    blendWeights /= (blendWeights.x + blendWeights.y + blendWeights.z);

    const float textureScale = 0.5;
    vec2 coord1 = In.position.yz * textureScale;
    vec2 coord2 = In.position.zx * textureScale;
    vec2 coord3 = In.position.xy * textureScale;

    vec4 color1 = texture(verticalTexture, coord1);
    vec4 color2 = texture(horizontalTexture, coord2);
    vec4 color3 = texture(verticalTexture, coord3);

    vec2 bumpFetch1 = texture(verticalTextureNormals, coord1).xy - 0.5;
    vec2 bumpFetch2 = texture(horizontalTextureNormals, coord2).xy - 0.5;
    vec2 bumpFetch3 = texture(verticalTextureNormals, coord3).xy - 0.5;

    vec3 bump1 = vec3(0, bumpFetch1);
    vec3 bump2 = vec3(bumpFetch2.y, 0, bumpFetch2.x);
    vec3 bump3 = vec3(bumpFetch3, 0);

    vec3 DiffuseColor = color1.rgb * blendWeights.x +
                        color2.rgb * blendWeights.y +
                        color3.rgb * blendWeights.z;
    vec3 BlendedBump = bump1.xyz * blendWeights.x +
                       bump2.xyz * blendWeights.y +
                       bump3.xyz * blendWeights.z;
    

    //gBufferDiffuseSpecular = vec4(DiffuseColor, SpecularExponent);
    //gBufferNormals         = normalize(In.normal + BlendedBump);
    gBufferDiffuseSpecular = vec4(DiffuseColor, 32);
    gBufferNormals         = In.normal;
    gBufferPosition        = In.position;

    /*vec3 N = perturb_normal(In.normal, normalize(In.position - CameraPosition), In.position.xz / 2);
    vec3 D = texture(horizontalTexture, In.position.xz / 2).rgb;

    gBufferDiffuseSpecular = vec4(D, SpecularExponent);
    gBufferNormals = N;
    gBufferPosition = In.position;*/
}