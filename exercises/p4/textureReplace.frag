#version 420 core

uniform sampler2D texUnit;
uniform vec2 windowSize;

in vec2 texCoordFrag;
out vec4 fragColor;

void main() { 
    vec4 color = texture(texUnit, texCoordFrag);

    // convertir a gris
    float gray = 0.3 * color.r + 0.59 * color.g + 0.11 * color.b;
    fragColor = vec4(gray, gray, gray, color.a);

    //Reducci�n de resoluci�n espacial
    if (int(gl_FragCoord.y) % 4 == 0 || int(gl_FragCoord.y) % 4 == 1 ) {
    fragColor = fragColor * 0.2;
    }else if (int(gl_FragCoord.y) % 4 == 2) {
    fragColor = fragColor * 0.8;
    }

    //Vignetting
    // TODO: bucle	
    float r = 100; // radio
    float md = distance(vec2(r, r), vec2(0.0, 0.0)); // maximum distance

    float bld = distance(gl_FragCoord.xy, vec2(r, r)); // bottom left distance
    if (bld > r && gl_FragCoord.x < r && gl_FragCoord.y < r ) {
        float vignete = 1 - (bld - r) / (md - r);
        fragColor = fragColor * vignete;
    }

    float brd = distance(gl_FragCoord.xy, vec2(windowSize.x - r, r)); // bottom right distance
    if (brd > r && gl_FragCoord.x > windowSize.x - r && gl_FragCoord.y < r ) {
        float vignete = 1 - (brd - r) / (md - r);
        fragColor = fragColor * vignete;
    }
  
    float trd = distance(gl_FragCoord.xy, vec2(windowSize.x - r, windowSize.y - r)); // top right distance
    if (trd > r && gl_FragCoord.x > windowSize.x - r && gl_FragCoord.y > windowSize.y - r ) {
        float vignete = 1 - (trd - r) / (md - r);
        fragColor = fragColor * vignete;
    }
    
    float tld = distance(gl_FragCoord.xy, vec2(r, windowSize.y - r)); // top left distance
    if (tld > r && gl_FragCoord.x < r && gl_FragCoord.y > windowSize.y - r ) {
        float vignete = 1 - (tld - r) / (md - r);
        fragColor = fragColor * vignete;
    }
}
