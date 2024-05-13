#version 420

$GLMatrices
$Lights
$Material

in vec3 N;
in vec3 pos;
out vec4 fragColor;

// Suponiendo fuentes puntuales
vec4 iluminacion(vec3 pos, vec3 N, vec3 V) {
  // Componente emisiva del material
  vec4 color = emissive;
  for (int i = 0; i < lights.length(); i++) {
    // Si la fuente esta apagada, no tenerla en cuenta
    if (lights[i].enabled == 0)
      continue;
    // Vector iluminacion (desde vertice a la fuente)
    vec3 L = lights[i].directional == 1 ? normalize(vec3(lights[i].positionEye)) : normalize(vec3(lights[i].positionEye) - pos);
    // Multiplicador de la componente difusa
    float diffuseMult = max(dot(N, L), 0.0);
    float specularMult = 0.0;
    if (diffuseMult > 0.0) {
      // Multiplicador de la componente especular
      vec3 R = reflect(-L, N);
      specularMult = max(0.0, dot(R, V));
      specularMult = pow(specularMult, shininess);
    }

    color += lights[i].ambient * ambient +
             lights[i].diffuse * diffuse * diffuseMult +
             lights[i].specular * specular * specularMult;

    if (lights[i].directional == 1) continue; // si es direccional se ignora atenuación y foco
    // factor de atenuación
    float d = length(vec3(lights[i].positionEye) - pos);
    float attenuation = 1.0 / max(1.0, lights[i].attenuation.x +
                                      lights[i].attenuation.y * d +
                                      lights[i].attenuation.z * d * d);
    color *= attenuation;

    // efecto foco
    if(lights[i].spotCutoff < 180.0) {
      float mLD = max(dot(-L, lights[i].spotDirectionEye),0);
      // comprobamos si el vertice esta dentro del cono de luz
      if(mLD >= lights[i].spotCosCutoff) {
        float spot = pow(mLD, lights[i].spotExponent);
        color *= spot;
      }else{
        color *= 0.0;
      }
    }
  }
  return color;
}

void main() {
  // Renormalizar N
  vec3 Nn = normalize(N);
  // calcular L y V en el espacio de la camara
  vec3 V = normalize(-pos);
  // Calcular iluminacion
  fragColor = iluminacion(pos, Nn, V);
}
