#version 420
$GLMatrices
in vec4 position;
in vec3 normal;

// Posición de la fuente en el espacio de la cámara
uniform vec3 lightpos;
uniform vec4 skyColor;
uniform vec4 groundColor;
out vec4 fragColor;

void main()
{
  vec3 ecPosition = (modelviewMatrix * position).xyz;
  vec3 tnorm = normalize(normalMatrix * normal);
  vec3 lightVec = normalize(lightpos - ecPosition);
  float costheta = dot(tnorm, lightVec);
  float a = 0.5 + 0.5 * costheta;
    
  fragColor = mix(groundColor, skyColor, a);
  gl_Position =  modelviewprojMatrix * position;
}
