#version 420 core

layout (vertices = 4) out;

in VS_OUT {
	vec2 textureCoord;
} from_vs[];

out TESC_OUT {
	vec2 textureCoord;
} to_tese[];


$GLMatrices

float calculateTessellationLevel(float distanceToCamera) {
	// Calculate the tessellation level based on the distance to the camera
	if (distanceToCamera < 5) {
		return 16;
	} else if (distanceToCamera < 10) {
		return 8;
	} else if (distanceToCamera < 20) {
		return 6;
	}	else {
		return 4;
	}
}

void main() {
	// Propagamos la posicion y la coordenada de textura al shader de evaluacion
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	to_tese[gl_InvocationID].textureCoord = from_vs[gl_InvocationID].textureCoord;

	// TODO: Se esta usando el clasificador precise bien?

	// inner tesellation
	if (gl_InvocationID == 0) {
		vec4 viewPosition = viewMatrix * modelMatrix * gl_in[0].gl_Position;
		precise float distanceToCamera = distance(viewPosition.xyz, vec3(0.0, 0.0, 0.0));
		float tessellationLevel = calculateTessellationLevel(distanceToCamera);

		gl_TessLevelInner[0] = tessellationLevel;
		gl_TessLevelInner[1] = tessellationLevel;
	}

	// outer tessellation
	for (int i = 0; i < 4; i++) {
		int next = (i + 1) % 4;

		vec4 pos1 = viewMatrix * modelMatrix * gl_in[i].gl_Position;
		vec4 pos2 = viewMatrix * modelMatrix * gl_in[next].gl_Position;

		precise float dis1  = distance(pos1.xyz, vec3(0.0, 0.0, 0.0));
		precise float dis2  = distance(pos2.xyz, vec3(0.0, 0.0, 0.0));
		
		float distanceToCamera = max(dis1, dis2);
		gl_TessLevelOuter[next] = calculateTessellationLevel(distanceToCamera);
	}
}
