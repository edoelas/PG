#version 420

$GLMatrices

layout(triangles) in;
layout(line_strip, max_vertices = 14) out;

// Color de la normal de vértice
uniform vec4 vertNColor;
// Color de la normal de cara
uniform vec4 faceNColor;
// Color de la tangente
uniform vec4 tangentColor;

uniform float normalLength;
uniform bool showFaceNormal;
uniform bool showVertexNormal;
uniform bool showTangent;

in vec3 ecnormal[];
in vec3 ectangent[];

out vec4 fragColor;

void main() {
  // Calculando la normal en el espacio de la cámara
  vec3 ab = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  vec3 ac = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
  vec3 fnormal = normalize(cross(ab, ac));
  vec3 center = (gl_in[0].gl_Position.xyz + gl_in[1].gl_Position.xyz +
                 gl_in[2].gl_Position.xyz) / 3;
  // El polígono está de espaldas si el producto escalar de su normal y
  // del vector que va de la cámara al polígono es mayor o igual que cero
  if (dot(fnormal, center) < 0) {
    if (showFaceNormal) {
      gl_Position = projMatrix * vec4(center, 1.0);
      fragColor = faceNColor;
      EmitVertex();
      gl_Position = projMatrix * (vec4(center + fnormal * normalLength, 1.0));
      fragColor = faceNColor;
      EmitVertex();
      EndPrimitive();
    }

    if (showVertexNormal) {
      for (int i = 0; i < gl_in.length(); i++) {
        gl_Position = projMatrix * gl_in[i].gl_Position;
        fragColor = vertNColor;
        EmitVertex();
        gl_Position = projMatrix * (gl_in[i].gl_Position +
                                    vec4(ecnormal[i] * normalLength, 0.0));
        fragColor = vertNColor;
        EmitVertex();
        EndPrimitive();
      }
    }

    if (showTangent) {
      for (int i = 0; i < gl_in.length(); i++) {
        gl_Position = projMatrix * gl_in[i].gl_Position;
        fragColor = tangentColor;
        EmitVertex();
        gl_Position = projMatrix * (gl_in[i].gl_Position +
                                    vec4(ectangent[i] * normalLength, 0.0));
        fragColor = tangentColor;
        EmitVertex();
        EndPrimitive();
      }
    }
  }
}
