#version 410 core

in vec4 position;

void main()
{
	// Simplemente pasamos la posici�n al shader de control de teselaci�n
	gl_Position = position;
}
