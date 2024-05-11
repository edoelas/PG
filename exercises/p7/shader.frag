#version 420 core

in TESE_OUT {
	vec2 textureCoord;
};

uniform	sampler2D texUnitHeightMap;
uniform	sampler1D texUnitcolorScaleHM;
out vec4 finalColor;

void main() {
	// TODO: solucion palera a las cumbres moradas
	float height = texture(texUnitHeightMap, textureCoord).r * 0.99;
	vec4 colorScale = texture(texUnitcolorScaleHM, height);
	finalColor = colorScale;
}
