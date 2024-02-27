#version 420

$GLMatrices

/*

Este shader pasa al shader de fragmento cada triángulo de entrada, pero 
también dibuja sus proyecciones en planta, alzado y perfil.
Ejemplo usando 4 invocaciones.

*/

layout (triangles, invocations = 4) in;
layout (triangle_strip, max_vertices = 12) out;

uniform vec4 projColor = vec4(0.2, 0.2, 0.2, 1.0); // color de las proyecciones
in vec4 color[3];
out vec4 fragColor;

void main() {
	mat4 projviewMatrix = projMatrix * viewMatrix;
	for (int i = 0; i < gl_in.length(); i++) {
	    vec4 proj = vec4(1.0);
		if (gl_InvocationID < 3) {
		  proj[gl_InvocationID] = 0.0;
		  fragColor = projColor;
		} else 
		  fragColor = color[i];  
		gl_Position = projviewMatrix * (gl_in[i].gl_Position * proj);
		
		EmitVertex();
	}
	EndPrimitive();
}


