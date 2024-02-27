#version 420 core

$GLMatrices

in vec3 position;
in vec2 texCoord;

// Enviamos al shader de fragmento la coordenada de textura y qué capa de la textura utilizar (0 o 1)
out PerVertex {
vec2 texCoordFrag;
flat int whichTile;
};

void main()
{
	// Tomamos los tres bits menos significativos como la coordenada en X
	int x = gl_InstanceID & 7;
	// Y los siguientes tres bits como la coordenada en Y
	int y = gl_InstanceID >> 3;

	// Calculamos qué tile utilizar para generar un tablero de ajedrez
	whichTile = (x & 1) ^ (y & 1);

	// Propagamos la coordenada de textura
	texCoordFrag = texCoord;
	// Movemos la instancia a su sitio
	vec3 newpos = position - vec3(float(x) - 4.0 + 0.5, float(y) - 4.0 + 0.5 , 0.0);
	gl_Position = modelviewprojMatrix * vec4(newpos, 1.0) ;
}
