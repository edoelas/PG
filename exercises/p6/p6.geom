#version 420

$GLMatrices

layout (triangles) in;
layout (triangle_strip) out;
layout (max_vertices = 3) out;

in vec4 color[3];
out vec4 fragColor;

uniform float cylinderRadius; // radio del cilindro
uniform float cylinderHeight; // altura del cilindro
uniform vec3 cylinderAxis; // dirección del eje (vector unitario)
uniform vec3 cylinderPos; // centro de la base del cilindro

void main() {
    int contador = 0;

	for (int i = 0; i < gl_in.length(); i++) {
        gl_Position = gl_in[i].gl_Position;
        vec4 test_Position = inverse(viewMatrix) * inverse(projMatrix) * gl_Position;
        if (distance(cylinderPos, test_Position.xyz) < cylinderRadius) {
            contador++;
        }
	}

    for (int i = 0; i < gl_in.length(); i++) {
        gl_Position = gl_in[i].gl_Position;
		fragColor = color[i];

        if (contador == 3) {
            fragColor = vec4(0.0, 1.0, 0.0, 1.0);
        }else if (contador == 2) {
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
        EmitVertex();

    }
	
    EndPrimitive();
}