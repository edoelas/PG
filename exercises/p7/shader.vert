#version 420 core

in vec3 position;
in vec2 texCoord;

$GLMatrices

uniform	sampler2D texUnitHeightMap;

out VS_OUT {
	vec2 textureCoord;
} vs_out;

void main()
{
	float x = (gl_InstanceID % 64 + position.x) - 32;
	float y =  position.y;
	float z = (gl_InstanceID / 64 + position.z)-32;
	gl_Position = vec4(x, y, z, 1.0);

	float tex_x = (gl_InstanceID % 64 + position.x) / 64;
	float tex_y = (gl_InstanceID / 64 + position.z) / 64;
	vs_out.textureCoord = vec2(tex_x, tex_y);
}
