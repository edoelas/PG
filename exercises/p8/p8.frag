#version 420 core

$Lights


layout (binding=$TEXDIFF) uniform sampler2D colores;
layout (binding=$TEXNORM) uniform sampler2D normales;
layout (binding=$TEXSPEC) uniform sampler2D brillos;
//layout (binding=$TEXHEIGHT) uniform sampler2D alturas;

uniform bool useParallax;

in vec2 TexCoord;
in vec3 L;
in vec3 V;

out vec4 fragColor;

vec4 iluminacion(vec3 L, vec3 N, vec3 V) {
    vec4 color = vec4(1.0,1.0,1.0, 1.0);
    // Si la fuente esta apagada, no tenerla en cuenta
    if (lights[i].enabled == 0)
        continue;
    // Vector iluminacion (desde vertice a la fuente)
//    vec3 L = normalize(vec3(lights[i].positionEye) - pos);
    // Multiplicador de la componente difusa
    float diffuseMult = max(dot(N, L), 0.0);
    float specularMult = 0.0;
    if (diffuseMult > 0.0) {
      // Multiplicador de la componente especular
      vec3 R = reflect(-L, N);
      specularMult = max(0.0, dot(R, V));
      specularMult = pow(specularMult, 0);  //TODO: shininess
    }

    color += lights[i].ambient * 0 +
             lights[i].diffuse * 0 * diffuseMult +
             lights[i].specular * 1 * specularMult;

    // factor de atenuación
//    float d = length(vec3(lights[i].positionEye) - pos);
//    float attenuation = 1.0 / max(1.0, lights[i].attenuation.x +
//                                      lights[i].attenuation.y * d +
//                                      lights[i].attenuation.z * d * d);
//    color *= attenuation;
  
  return color;
}

void main()
{
//    // Renormalizar N
//    vec3 Nn = normalize(N);
//    // calcular L y V en el espacio de la camara
//    vec3 V = normalize(-pos);
//    // Calcular iluminacion
//    fragColor = iluminacion(pos, Nn, V);

	if (useParallax) {
		fragColor = texture(normales, TexCoord);
	} else {
		vec4 c = texture(colores, TexCoord);
		vec4 n = texture(normales, TexCoord);
		vec4 b = texture(brillos, TexCoord);

		vec3 nL = normalize(L);
        vec3 nV = normalize(V);
		fragColor = iluminacion(nL, normalize(n.xyz*2-vec3(1)), nV) * c;
	}
}
