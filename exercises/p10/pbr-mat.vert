#version 430 core

$GLMatrices

in vec4 position;
in vec3 normal;
in vec2 texcoords;
in vec3 tangent;

out vec2 TexCoords;
out vec3 ecPosition;
out vec3 ecNormal;

void main()
{
	vec3 T = vec3(modelviewMatrix * vec4(tangent, 0.0));
	vec3 N = normalMatrix * normal;
	vec3 B = cross(N, T);
	mat3 R = transpose(mat3(T, B, N));

    TexCoords = texcoords;
    ecPosition = R * vec3(modelviewMatrix * position);
    ecNormal = R * normalMatrix * normal;   

    gl_Position =  modelviewprojMatrix * position;
}