
#version 420 core

uniform	sampler2DArray texUnit;

in PerVertex {
  vec2 texCoordFrag;
  flat int whichTile;
} from_vs;

out vec4 fragColor;

void main()
{
	fragColor = texture(texUnit, vec3(from_vs.texCoordFrag, from_vs.whichTile));
}
