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
	vec3 T = vec3(modelviewMatrix * vec4(tangent, 0.0));
	vec3 N = normalMatrix * normal;
	vec3 B = cross(N, T);
	mat3 R = transpose(mat3(T, B, N));

	TexCoord = vec2(texCoord);
	vec3 Lc = vec3(lights[0].positionEye - modelviewMatrix * position);
	L = R * Lc;
//	vec3 Vc = vec3(- modelviewMatrix * position);
//	V = R * Vc;
	V = vec3(- modelviewMatrix * position);
	gl_Position = modelviewprojMatrix * position;
}	
