#version 420 core

$GLMatrices

in vec3 position;
in vec2 texCoord;

// Pasamos la misma coordenada de textura, cambiando el tipo de interpolación
smooth out vec2 texCoordSmooth;
flat out vec2 texCoordFlat;
noperspective out vec2 texCoordNoPers;

void main()
{
	texCoordSmooth = texCoord;
	texCoordFlat = texCoord;
	texCoordNoPers = texCoord;
	gl_Position = modelviewprojMatrix *  vec4(position, 1.0) ;
}