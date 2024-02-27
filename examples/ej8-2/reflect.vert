#version 420

$GLMatrices

// Indica si debemos trabajar en un espacio levógiro o no
uniform bool levo = true;

in vec3 normal;
in vec4 position;

// Vector en la dirección del reflejo
out vec3 reflectDir;

void main(void)
{
  vec3 N = normalize(normalMatrix * normal);
  vec4 V = modelviewMatrix * position;
  // Para una matriz de rotación, su inversa y su transpuesta
  // son iguales
  reflectDir = transpose(mat3(viewMatrix)) * reflect(V.xyz, N);
  if (levo) reflectDir *= vec3(1.0, 1.0, -1.0);  
  gl_Position = projMatrix * V;
}
