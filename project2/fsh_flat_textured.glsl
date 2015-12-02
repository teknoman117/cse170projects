# version 400

in vec4 Color;
in vec3 Norm;

uniform vec4 kd;

uniform sampler2D diffuse;

out vec4 fColor;

void main() 
{
    vec3 n = normalize(Norm);
    vec2 s;
    s.x = ((atan(-n.z,n.x) / 3.14159f)+1.0f)/2.0f;
    s.y = (n.y+1.0f)/2.0f;
    
    fColor = texture(diffuse,s) * kd;
}
