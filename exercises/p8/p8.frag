#version 420 core

$Lights


layout (binding=$TEXDIFF) uniform sampler2D colores;
layout (binding=$TEXNORM) uniform sampler2D normales;
layout (binding=$TEXSPEC) uniform sampler2D brillos;
layout (binding=$TEXHEIGHT) uniform sampler2D alturas;

uniform bool useParallax;
uniform bool useFakeNormals;
uniform bool showNormalMap;
uniform float parallaxBias;
uniform float parallaxScale;

uniform float size;
// uniform float offset;
uniform float scale;

uniform float matAmbient;
uniform float matDiffuse;
uniform float matSpecular;
uniform float matShininess;

in vec2 TexCoord;
in vec3 L;
in vec3 V;

out vec4 fragColor;

vec4 iluminacion(vec3 L, vec3 N, vec3 V, float d, vec4 b) {
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	// Multiplicador de la componente difusa
	float diffuseMult = max(dot(N, L), 0.0); // abs??
	float specularMult = 0.0;
	if (diffuseMult > 0.0) {
		// Multiplicador de la componente especular
		vec3 R = reflect(-L, N);
		specularMult = max(0.0, dot(R, V));// abs??
		specularMult = pow(specularMult, matShininess) * b.x; // shininess
	}

	color += lights[0].ambient * matAmbient +
			 lights[0].diffuse * matDiffuse * diffuseMult +
			 lights[0].specular * matSpecular * specularMult;

	//factor de atenuación
	float attenuation = 1.0 / max(1.0, lights[0].attenuation.x +
									  lights[0].attenuation.y * d +
									  lights[0].attenuation.z * d * d);
	color *= attenuation;   

	return color;
}

void main()
{
	vec4 b = texture(brillos, TexCoord);

	// Parallax mapping
	vec2 coord;
	if (useParallax) {
		float h = texture(alturas, TexCoord).r * parallaxScale - parallaxBias;
		coord = TexCoord + h * V.xy;
	} else {
		coord  = TexCoord;
	}

	vec4 c = texture(colores, coord);
	vec4 n;
	if( useFakeNormals ){
		// estimate normal map from height map
		const vec2 size = vec2(size,0);
		const ivec3 off = ivec3(-2,0,2);
		// const ivec3 off = ivec3(-offset,0,offset);
		vec4 wave = texture(alturas, coord);
		float s11 = wave.x * scale;
		float s01 = textureOffset(alturas, coord, off.xy).x * scale;
		float s21 = textureOffset(alturas, coord, off.zy).x * scale;
		float s10 = textureOffset(alturas, coord, off.yx).x * scale;
		float s12 = textureOffset(alturas, coord, off.yz).x * scale;
		vec3 va = vec3(size.xy,s21-s01);
		vec3 vb = vec3(size.yx,s12-s10);
		n = vec4( cross(va,vb), s11 );
		n = normalize(n);
		n = (n+1) / 2;
	} else {
		n = texture(normales, coord);
	}

	vec3 nL = normalize(L);
	vec3 nV = normalize(V);
	vec3 nN = normalize(n.xyz * 2.0 - 1.0);

	if(showNormalMap)
		fragColor = n;
	else
		fragColor = iluminacion(nL, nN, nV, length(L), b) * c;
	
}
