# version 400

in vec3 Pos;
in vec4 Color;
in vec3 Normal;
out vec4 fColor;

void main() 
{ 
	vec3 l = normalize(vec3(0,2,0) - Pos);
	vec3 n = normalize(Normal);

	vec4 d = Color * dot(l,n);
	if(dot(l,n)<0)
	{
	    d = vec4(0,0,0,1);
	}

    fColor = vec4(d.xyz + 0.15*Color.xyz, 1.0);
} 
