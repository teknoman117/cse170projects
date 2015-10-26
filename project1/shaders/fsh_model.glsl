#version 400

// Input fragment color from vertex lighting stage
in vec4 fragment_color_in;

// Input texture coordinate
in vec2 texture_coordinates;

// Are we textured
uniform int textured;

//Texture input
uniform sampler2D diffuse;

// Color output
out vec4 fragmentColor;

void main ()
{
	// Calculate the base color for the fragment (lighting * texture)
	vec4 finalColor = fragment_color_in;
    
    // Are we textured
    if(textured > 0)
    {
        finalColor = finalColor * texture(diffuse, texture_coordinates);
    }
    
    fragmentColor = vec4(finalColor.rgb, 1.0);
}

