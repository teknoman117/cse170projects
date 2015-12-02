#version 400

in vec3 Norm;
in vec3 l;

/*uniform vec4 la;
uniform vec4 ld;
uniform vec4 ls;

uniform vec4 ka;
uniform vec4 kd;
uniform vec4 ks;

uniform float sh;

uniform sampler2D diffuse;*/

out vec4 FragColor;

void main()
{
    //vec3 p = Pos;
    
    // Compute the lighting vectors
    //vec3 l = normalize(lPos - p);
    vec3 n = normalize(Norm);
    /*vec3 v = normalize(vec3(0,0,2) - p);
    vec3 r = reflect(-l, n);
    
    // Compute ambient lighting
    vec4 a = la * ka;
    vec4 d = ld * kd * dot(l, n);
    vec4 s = ls * ks * pow(max(dot(r, v), 0), sh);
    if(dot(l,n) < 0)
    {
        s = vec4(0,0,0,1);
        d = vec4(0,0,0,1);
    }
    
    // Compute texture coordinater
    vec3 raw = normalize(RawNorm);
    vec2 tex;
    tex.x = ((atan(-raw.z,raw.x) / 3.14159f)+1.0f)/2.0f;
    tex.y = (raw.y+1.0f)/2.0f;
    
    // Output the final color
    FragColor = texture(diffuse, tex) * (a + d + s);*/
    FragColor = vec4(vec3(1,1,1) * dot(l,n), 1);
    //FragColor = vec4(normalize(vec3(n.r, 0, n.b)), 1);
}
