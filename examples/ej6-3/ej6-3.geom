#version 420

$GLMatrices

/*

Este shader pasa al shader de fragmento cada triángulo de entrada, pero 
también dibuja sus proyecciones en planta, alzado y perfil.

*/

layout (triangles) in;
layout (triangle_strip, max_vertices = 12) out;

uniform vec4 projColor = vec4(0.2, 0.2, 0.2, 1.0); // color de las proyecciones
in vec4 color[3];
out vec4 fragColor;

void main() {
	mat4 projviewMatrix = projMatrix * viewMatrix;
	// Geometría "normal"
	for (int i = 0; i < gl_in.length(); i++) {
		gl_Position = projviewMatrix * gl_in[i].gl_Position;
		fragColor = color[i];
		EmitVertex();
	}
	EndPrimitive();
	
	
	// Planta
	for (int i = 0; i < gl_in.length(); i++) {
		gl_Position =  projviewMatrix * (gl_in[i].gl_Position * vec4(1.0, 0.0, 1.0, 1.0));
		fragColor = projColor;
		EmitVertex();
	}
	EndPrimitive();

	// Perfil
	for (int i = 0; i < gl_in.length(); i++) {
		gl_Position =  projviewMatrix * (gl_in[i].gl_Position * vec4(0.0, 1.0, 1.0, 1.0));
		fragColor = projColor;
		EmitVertex();
	}
	EndPrimitive();

	// Alzado
	for (int i = 0; i < gl_in.length(); i++) {
		gl_Position =  projviewMatrix * (gl_in[i].gl_Position * vec4(1.0, 1.0, 0.0, 1.0));
		fragColor = projColor;
		EmitVertex();
	}
	EndPrimitive();

}


