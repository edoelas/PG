#version 420 core

$Lights

layout (binding=$TEXDIFF) uniform sampler2D colores;
layout (binding=$TEXNORM) uniform sampler2D normales;
layout (binding=$TEXSPEC) uniform sampler2D brillos;
// Descomenta este sampler cuando estés preparado para usarlo
//layout (binding=$TEXHEIGHT) uniform sampler2D alturas;

uniform bool useParallax;

in vec2 TexCoord;
out vec4 fragColor;

void main()
{
	// Este código accede a los uniforms para que el compilador no los
	// descarte y pueda arrancar el programa. 
	// NO HACE NADA QUE TE PUEDA SERVIR. Sustitúyelo por tu código.

	if (useParallax) {
		fragColor = vec4(1.0, 0.0, 0.0, 1.0);
	} else {
		vec4 c = texture(colores, TexCoord);
		vec4 n = texture(normales, TexCoord);
		vec4 b = texture(brillos, TexCoord);
		if (lights[0].positionEye.y > 2.0) {
			fragColor = c * n * b;
		} else
			fragColor = c;
	}
}
