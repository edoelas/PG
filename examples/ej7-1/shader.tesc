#version 410 core

layout (vertices = 4) out;

// Niveles de teselación pasados desde la aplicación
uniform vec4 levelOuter;
uniform vec2 levelInner;

void main() {
	// De nuevo propagamos la posición al shader de evaluación
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	// Si estamos procesando el primer vértice del parche, establecer los niveles 
	// de teselación
	if (gl_InvocationID == 0) {
		gl_TessLevelOuter[0] = levelOuter[0];
		gl_TessLevelOuter[1] = levelOuter[1];
		gl_TessLevelOuter[2] = levelOuter[2];
		gl_TessLevelOuter[3] = levelOuter[3];

		gl_TessLevelInner[0] = levelInner[0];
		gl_TessLevelInner[1] = levelInner[1];
	}
}
