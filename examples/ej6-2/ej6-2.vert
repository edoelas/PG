#version 420

$GLMatrices

in vec4 position;
in vec3 normal;


// Posición de la fuente en el espacio de la cámara
uniform vec3 lightpos;
// Color difuso del objeto
// Los uniforms se pueden inicializar en el shader. Si la aplicación no escribe en la variable
// (usando, por ejemplo, glUniform4f), se usará este valor.
uniform vec4 diffuseColor = vec4(0.8, 0.3, 0.3, 1.0);

out vec4 color;


void main() {
	// Normal del vértice en el espacio de la cámara
	vec3 eN = normalize(normalMatrix *normal);
	// Vértice en el espacio de la cámara
	vec4 eposition=modelviewMatrix * position;
	// Cálculo de la iluminación
	color = max(0.0, dot(eN, normalize(lightpos-eposition.xyz))) * diffuseColor;
	gl_Position = eposition;
}
