#version 420 core

uniform sampler2D texUnit;

in vec2 texCoordFrag;
out vec4 fragColor;

void main() { 
  vec4 color = texture(texUnit, texCoordFrag);

  // convertir a gris
  float gray = 0.3 * color.r + 0.59 * color.g + 0.11 * color.b;
  fragColor = vec4(gray, gray, gray, color.a);

  //Reducción de resolución espacial
  if (int(gl_FragCoord.y) % 4 == 0 || int(gl_FragCoord.y) % 4 == 1 ) {
    fragColor = fragColor * 0.2;
  }else if (int(gl_FragCoord.y) % 4 == 2 || int(gl_FragCoord.y) % 4 == 3 ) {
	fragColor = fragColor * 0.8;
  }

  //Vignetting
  // TODO: tal vez se pueda hacer con un bucle	
  // TODO: las esquinas estan achatadas
  float r = 0.2; // radio
  float md = distance(vec2(r, r), vec2(0.0, 0.0)); // maximum distance

  float bld = distance(texCoordFrag.xy, vec2(r, r)); // bottom left distance
  if (bld > r && texCoordFrag.x < r && texCoordFrag.y < r ) {
	float vignete = 1 - (bld - r) / (md - r);
	fragColor = fragColor * vignete;
  }

  float brd = distance(texCoordFrag.xy, vec2(1.0 - r, r)); // bottom right distance
  if (brd > r && texCoordFrag.x > 1.0 - r && texCoordFrag.y < r ) {
	  float vignete = 1 - (brd - r) / (md - r);
	  fragColor = fragColor * vignete;
  }
  
  float trd = distance(texCoordFrag.xy, vec2(1.0 - r, 1.0 - r)); // top right distance
  if (trd > r && texCoordFrag.x > 1.0 - r && texCoordFrag.y > 1.0 - r ) {
	  float vignete = 1 - (trd - r) / (md - r);
	  fragColor = fragColor * vignete;
  }

  float tld = distance(texCoordFrag.xy, vec2(r, 1.0 - r)); // top left distance
  if (tld > r && texCoordFrag.x < r && texCoordFrag.y > 1.0 - r ) {
	  float vignete = 1 - (tld - r) / (md - r);
	  fragColor = fragColor * vignete;
  }
}
