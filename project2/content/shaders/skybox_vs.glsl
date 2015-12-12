#version 410
uniform mat4 uProjectionMatrix;
uniform mat4 uWorldToCameraMatrix;

layout(location = 0) in vec4 aPosition;

smooth out vec3 eyeDirection;

void main() 
{
    mat4 inverseProjection = inverse(uProjectionMatrix);
    mat3 inverseModelview = transpose(mat3(uWorldToCameraMatrix));
    vec3 unprojected = (inverseProjection * aPosition).xyz;
    eyeDirection = inverseModelview * unprojected;

    gl_Position = aPosition;
} 