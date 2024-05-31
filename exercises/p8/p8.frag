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

vec4 iluminacion(vec3 L, vec3 N, vec3 V, vec4 color, float d) {
    
    // Multiplicador de la componente difusa
    float diffuseMult = max(dot(N, L), 0.0);
    float specularMult = 0.0;
    if (diffuseMult > 0.0) {
      // Multiplicador de la componente especular
      vec3 R = reflect(-L, N);
      specularMult = max(0.0, dot(R, V));
      specularMult = pow(specularMult, 0.5);  //TODO: shininess
    }

    color += lights[0].ambient * 0 +
             lights[0].diffuse * 0.1 * diffuseMult +
             lights[0].specular * 0.2 * specularMult;

    //factor de atenuación
    float attenuation = 1.0 / max(1.0, lights[0].attenuation.x +
                                      lights[0].attenuation.y * d +
                                      lights[0].attenuation.z * d * d);
    color *= attenuation;   
  
  return color;
}

void main()
{
	if (useParallax) {
		fragColor = texture(normales, TexCoord);
	} else {
		vec4 c = texture(colores, TexCoord);
		vec4 n = texture(normales, TexCoord);
		vec4 b = texture(brillos, TexCoord);

		vec3 nL = normalize(L);
        vec3 nV = normalize(V);
        vec3 nN = normalize(n.xyz * 2.0 - 1.0);
		fragColor = iluminacion(nL, nN, nV, c, 1);
	}
}
