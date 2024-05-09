#version 420 core

layout (vertices = 4) out;

in VS_OUT {
	vec2 textureCoord;
} from_vs[];

out TESC_OUT {
	vec2 textureCoord;
} to_tese[];


$GLMatrices

void main() {
	// Propagamos la posición y la coordenada de textura al shader de evaluación
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	to_tese[gl_InvocationID].textureCoord = from_vs[gl_InvocationID].textureCoord;

	/*
		Calcula aquí los niveles de teselación
	*/
	if (gl_InvocationID == 0) {
		gl_TessLevelOuter[0] = 64.0;
		gl_TessLevelOuter[1] = 64.0;
		gl_TessLevelOuter[2] = 64.0;
		gl_TessLevelOuter[3] = 64.0;

		gl_TessLevelInner[0] = 64.0;
		gl_TessLevelInner[1] = 64.0;
	}
}
