#version 420

$Lights
$Material

uniform sampler2DShadow depthTexture1;
uniform sampler2DShadow depthTexture2;

in vec3 N1, V1, N2, V2;
in vec3 epos1, epos2;
in vec4 spos1, spos2;

out vec4 fragColor;

// Sólo se tiene en cuenta la primera fuente, y se considera que 
// es puntual.
vec4 iluminacion(vec3 pos, vec3 N, vec3 V, float f, int id) {
  vec4 color = emissive + lights[id].ambient * ambient;
  // Vector iluminación (desde vértice a la fuente)
  vec3 L = normalize(vec3(lights[id].positionEye) - pos);
  // Multiplicador de la componente difusa
  float diffuseMult = max(dot(N,L), 0.0);
  float specularMult = 0.0;
  if (diffuseMult > 0.0) {
    // Multiplicador de la componente especular
    vec3 R = reflect(-L, N);
    specularMult = max(0.0, dot(R, V));
    specularMult = pow(specularMult, shininess);
  }
  color += f * (lights[id].diffuse * diffuse * diffuseMult +
         lights[id].specular * specular * specularMult);
  return color;
}

void main() {
  float f1 = textureProj(depthTexture1, spos1);
  vec4 frag1 = iluminacion(epos1, normalize(N1), normalize(V1), f1, 0);

  float f2 = textureProj(depthTexture2, spos2);
  vec4 frag2 = iluminacion(epos2, normalize(N2), normalize(V2), f2, 1);

  fragColor = frag1 + frag2;
}
