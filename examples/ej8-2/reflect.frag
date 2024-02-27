#version 420

uniform samplerCube envmap;

in vec3 reflectDir;
out vec4 finalColor;

void main(void)
{
  // El color final viene dado por el color de la textura
  // en la dirección del reflejo
  finalColor = texture(envmap, reflectDir);
}
