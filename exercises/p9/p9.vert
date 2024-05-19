#version 420

$GLMatrices

uniform mat4 shadowMatrix1;
uniform mat4 shadowMatrix2;

in vec4 position;
in vec3 normal;

out vec3 N1, V1, N2, V2;
out vec3 epos1, epos2;
out vec4 spos1, spos2;

void main() {
  // Normal en el espacio de la c�mara
  N1 = normalize(normalMatrix * normal);
  // V�rtice en el espacio de la c�mara
  epos1 = vec3(modelviewMatrix * position);
  // Posici�n del v�rtice en el espacio de clip escalado de la fuente
  spos1 = shadowMatrix1 * modelMatrix * position;
  // Vector vista (desde v�rtice a la c�mara)
  V1 = normalize(-epos1);

    // Normal en el espacio de la c�mara
  N2 = normalize(normalMatrix * normal);
  // V�rtice en el espacio de la c�mara
  epos2 = vec3(modelviewMatrix * position);
  // Posici�n del v�rtice en el espacio de clip escalado de la fuente
  spos2 = shadowMatrix1 * modelMatrix * position;
  // Vector vista (desde v�rtice a la c�mara)
  V2 = normalize(-epos2);

  gl_Position = modelviewprojMatrix * position;
}
