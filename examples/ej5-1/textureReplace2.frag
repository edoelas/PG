#version 420 core

uniform	sampler2D texUnit;

// Dependiendo del valor de este uniform...
uniform int which;  // 0: smooth, 1: flat, 2: no perspective

// ...usaremos una de estas coordenadas de textura
smooth in vec2 texCoordSmooth;
flat in vec2 texCoordFlat;
noperspective in vec2 texCoordNoPers;

out vec4 fragColor;

void main()
{
	switch (which) {
	case 0:
		fragColor = texture(texUnit, texCoordSmooth);
		break;
	case 1:
		fragColor = texture(texUnit, texCoordFlat);
		break;
	case 2:
		fragColor = texture(texUnit, texCoordNoPers);
		break;
	default:
		fragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
}
