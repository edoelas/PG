#version 420

$GLMatrices

in vec4 position;
in vec3 normal;
in vec3 tangent;

out vec3 ecnormal;
out vec3 ectangent;

void main() {
	// Normal del vértice en el espacio de la cámara
	ecnormal = normalize(normalMatrix *normal);
	// Tangente en el espacio de la cámara
	ectangent = normalize(normalMatrix * tangent);
	// Vértice en el espacio de la cámara
	gl_Position = modelviewMatrix * position;
}
