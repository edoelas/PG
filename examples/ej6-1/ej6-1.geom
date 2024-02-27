
#version 420

/* 

Shader de geometría identidad: entra un triángulo, sale un triángulo.

*/

layout (triangles) in;
layout (triangle_strip) out;
layout (max_vertices = 3) out;

in vec4 color[3];
out vec4 fragColor;

void main() {
	for (int i = 0; i < gl_in.length(); i++) {
		gl_Position = gl_in[i].gl_Position;
		fragColor = color[i];
		EmitVertex();
	}
	EndPrimitive();
}