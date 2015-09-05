# version 400

in vec4 Color;
out vec4 fColor;

void main() 
{
    //const vec3 lightPos = vec3(5, 5, 5);
    //vec3 deerrr = normalize(lightPos);

    //float l = dot(Color.xyz, deerrr);

    //fColor = vec4(l, l, l, 1.0);
    fColor = abs(Color);
} 
