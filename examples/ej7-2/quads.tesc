#version 410 core

uniform vec4 outerLevel;
uniform vec2 innerLevel;

layout (vertices = 4) out;

void main() {
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	if (gl_InvocationID == 0) {
		gl_TessLevelOuter[0] = outerLevel.x;
		gl_TessLevelOuter[1] = outerLevel.y;
		gl_TessLevelOuter[2] = outerLevel.z;
		gl_TessLevelOuter[3] = outerLevel.w;

		gl_TessLevelInner[0] = innerLevel.x;
		gl_TessLevelInner[1] = innerLevel.y;
	}
}
