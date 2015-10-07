# version 400

uniform sampler2D Texture;
uniform float     BlendFactor;

in  vec4 Color;
in  vec2 TexCoord;

out vec4 fColor;

void main() 
{
     //fColor = (BlendFactor * Color) + ((1.0-BlendFactor) * texture(Texture, TexCoord));
    fColor = mix(Color, texture(Texture, TexCoord), BlendFactor);
}
