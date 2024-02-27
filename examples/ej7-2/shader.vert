#version 410 core

in vec3 position;

// Este shader es común a todos los programas. Simplemente pasa las posiciones
void main()
{
	gl_Position = vec4(position,1.0);
}
