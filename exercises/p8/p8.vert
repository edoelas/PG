#version 420 core

$GLMatrices
$Lights

//struct LightSource {
//	vec4 ambient, diffuse, specular;
//	vec4 positionWorld, positionEye; // Posición de la luz (mundo y cámara)
//	vec3 spotDirectionWorld; // Dirección del foco en el S.C. del mundo
//	int directional; // Es direccional?
//	vec3 spotDirectionEye; // Dirección del foco en el S.C. de la cámara
//	int enabled; // Está encendida?
//	float spotExponent, spotCutoff, spotCosCutoff; // Datos del foco
//	vec3 attenuation; // kc, kl, kq
//};

// Atributos del vértice (sistema de coordenadas del modelo)
in vec4 position;
in vec2 texCoord;
in vec3 normal;
in vec3 tangent;

// Añade las variables out que necesites
out vec2 TexCoord;
out vec3 L;
out vec3 V;

void main()
{

	L = vec3(lights[0].positionEye - modelviewMatrix * position);
	V = vec3(- modelviewMatrix * position);
	TexCoord = vec2(texCoord);
	gl_Position = modelviewprojMatrix * position;

}
