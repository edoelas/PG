#version 420

/* Ejemplo adaptado de la Superbiblia, 6a edición (pp. 108-118) */

layout (std140) uniform TransformBlock {
  float scale;
  vec3 translation;
  float rotation[3];
  mat4 proj_matrix;
};


in vec3 position;
in vec4 vertcolor;

out vec4 fragcolor;

void main()
{
  float c0 = cos(rotation[0]), s0 = sin(rotation[0]);
  mat3 Rx = mat3(1, 0, 0,   0, c0, s0,   0, -s0, c0);

  float c1 = cos(rotation[1]), s1 = sin(rotation[1]);
  mat3 Ry = mat3(c1, 0, -s1,  0, 1, 0,  s1, 0, c1);

  float c2 = cos(rotation[2]), s2 = sin(rotation[2]);
  mat3 Rz = mat3(c2, s2, 0,  -s2, c2, 0,  0, 0, 1);
  
  mat3 rot=Rx * Ry * Rz;
  fragcolor = vertcolor;
  gl_Position = proj_matrix * vec4((rot * (position*scale)) + translation, 1.0);
}
