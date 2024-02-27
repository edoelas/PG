#version 420

uniform vec4 baseColor;
uniform float specularPercent;
uniform float diffusePercent;

uniform samplerCube specularMap;
uniform samplerCube diffuseMap;

in vec3 reflectDir;
in vec3 normalDir;

out vec4 finalColor;

void main(void)
{
	vec4 diffColor = texture(diffuseMap, normalDir);
	vec4 specColor = texture(specularMap, reflectDir);
	vec4 color = mix(baseColor, diffColor*baseColor, diffusePercent);
	color = mix(color, specColor + color, specularPercent);
	finalColor = vec4(color.xyz, 1.0);
}
