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

// Create a function to check if a vertex is inside the cylinder
bool isInsideCylinder(vec3 vertex) {
    //return distance(cylinderPos, test_Position.xyz) < cylinderRadius
    vec3 v = vertex - cylinderPos;
    float projection = dot(v, cylinderAxis);
    vec3 projectedPoint = cylinderPos + projection * cylinderAxis;
    float distanceToAxis = distance(vertex, projectedPoint);
    return distanceToAxis < cylinderRadius && projection > 0.0 && projection < cylinderHeight;
}

void main() {
    int contador = 0;

	for (int i = 0; i < gl_in.length(); i++) {
        gl_Position = gl_in[i].gl_Position;
        vec4 test_Position = inverse(viewMatrix) * inverse(projMatrix) * gl_Position;
        if (isInsideCylinder(test_Position.xyz)) {
            contador++;
        }
	}

    for (int i = 0; i < gl_in.length(); i++) {
        gl_Position = gl_in[i].gl_Position;
        switch (contador) {
        case 0:
            fragColor = color[i];
            EmitVertex();
            break;
        case 1:
            fragColor = vec4(0.0, 1.0, 0.0, 1.0);
            EmitVertex();
            break;
        case 2:
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
            EmitVertex();
            break;
        }

    }
	
    EndPrimitive();
}

