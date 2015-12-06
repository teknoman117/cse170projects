/**
 * HDR Tonemap and Gamma correction shader implemented based on Frictional Games blog posts
 * http://frictionalgames.blogspot.com/2012/09/tech-feature-hdr-lightning.html
 */

#version 410

uniform sampler2D inputTexture;
uniform float     correctionFactor;

smooth in vec2 textureCoordinate;
out vec4 fragmentColor;

vec3 Uncharted2Tonemap(vec3 x)
{
	const float A = 0.15;
	const float B = 0.50;
	const float C = 0.10;
	const float D = 0.20;
	const float E = 0.02;
	const float F = 0.30;

	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

void main()
{
	// Tonemap
	vec4 colorHDR = texture(inputTexture, textureCoordinate);
	vec3 colorTonemapped = Uncharted2Tonemap(colorHDR.rgb);

	// Gamma correct
	vec3 correction = vec3(correctionFactor, correctionFactor, correctionFactor);
	vec3 colorGammaCorrected = pow(colorTonemapped, correction);

	// Output to screen
	fragmentColor = vec4(colorGammaCorrected, 1);
}
