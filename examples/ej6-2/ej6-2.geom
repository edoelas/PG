

#version 420

$GLMatrices

/*

Este shader descarta el triángulo de entrada si no está orientado hacia la esfera

*/

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// Posición de la esfera en el espacio de la cámara
uniform vec4 sphere;

in vec4 color[3];
out vec4 fragColor;

void main() {
	// Calculando la normal en el espacio de la cámara
	// (los vértices ya están en ese espacio)
    vec3 ab = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 ac = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 normal = cross(ab, ac); // No hace falta normalizar
    
    // Vector desde el primer vértice a la esfera (espacio de la cámara)
    vec3 vt = vec3(sphere-gl_in[0].gl_Position); // No hace falta normalizar
    
    if (dot(vt, normal) > 0.0) {
		for (int i = 0; i < gl_in.length(); i++) {
			gl_Position = projMatrix * gl_in[i].gl_Position;
			fragColor = color[i];
			EmitVertex();
		}
		EndPrimitive();
	}
}


