#version 420

$GLMatrices
$Lights
$Material

in vec4 position;
in vec3 normal;

out vec3 N;
out vec3 pos;

void main() {
  N = normalize(normalMatrix * normal);
  pos = vec3(modelviewMatrix * position);
  gl_Position = modelviewprojMatrix * position;
}
