#version 420 core

$GLMatrices
$Lights

// Atributos del vértice (sistema de coordenadas del modelo)
in vec4 position;
in vec2 texCoord;
in vec3 normal;
in vec3 tangent;

// Añade las variables out que necesites
out vec2 TexCoord;

void main()
{
	TexCoord = vec2(texCoord);
	gl_Position = modelviewprojMatrix * position;
}
