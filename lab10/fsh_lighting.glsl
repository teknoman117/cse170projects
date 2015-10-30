#version 400

in vec3 Norm;
in vec4 Pos;
out vec4 FragColor;

uniform vec3 lPos;
uniform vec4 la;
uniform vec4 ld;
uniform vec4 ls;

uniform vec4 ka;
uniform vec4 kd;
uniform vec4 ks;

uniform float sh;

void main()
{
    vec3 p = Pos.xyz / Pos.w;
    
    // Compute the lighting vectors
    vec3 l = normalize(lPos - p);
    vec3 n = Norm;
    vec3 v = vec3(0,0,1);
    vec3 r = reflect(-l, n);
    
    // Compute ambient lighting
    vec4 a = la * ka;
    vec4 d = ld * kd * dot(l, n);
    vec4 s = ls * ks * pow(max(r.z, 0), sh);
    if(dot(l,n) < 0)
    {
        s = vec4(0,0,0,1);
        d = vec4(0,0,0,1);
    }
    
    FragColor = a + d + s;
}
