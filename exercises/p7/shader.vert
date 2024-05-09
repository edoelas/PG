#version 420 core

in vec3 position;
in vec2 texCoord;

out VS_OUT {
	vec2 textureCoord;
} vs_out;

void main()
{
	gl_Position = vec4(position, 1.0);
	vs_out.textureCoord = texCoord;
}
