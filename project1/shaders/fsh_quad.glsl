#version 400

in vec2 Tex;
in vec3 Norm;

uniform sampler2D diffuse;

out vec4 fragmentColor;

void main ()
{
    // future lighting shit
    
    fragmentColor = vec4(texture(diffuse, Tex).rgb, 1.0);
    //fragmentColor = vec4(1,1,1,1);
}