# version 400

in vec4 Color;
in vec3 Normal;

out vec4 fColor;

void main() 
{
    const vec3 lightPos = vec3(5.0f, 5.0f, 0.0f);
    vec3 surfaceToLight = normalize(lightPos);

    float diffuseCoefficient = max(0.0, dot(Normal, surfaceToLight));

    fColor = vec4(Color.xyz * diffuseCoefficient, 1.0);
} 
