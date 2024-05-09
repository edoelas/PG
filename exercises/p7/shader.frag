#version 420 core

in TESE_OUT {
	vec2 textureCoord;
};

uniform	sampler2D texUnitHeightMap;
out vec4 finalColor;


void main() {
	finalColor = texture(texUnitHeightMap, textureCoord).rrra; 
}
