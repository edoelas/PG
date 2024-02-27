#version 410 core

in vec4 position;

void main()
{
	// Simplemente pasamos la posición al shader de control de teselación
	gl_Position = position;
}
