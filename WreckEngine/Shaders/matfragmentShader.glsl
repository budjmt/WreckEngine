#version 400

uniform vec4 tint;

uniform sampler2D uniformTex;
in vec4 fragPos;
in vec2 fragUV;
in vec4 fragNormal;

in float ftime;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 fragPosition;
layout (location = 2) out vec4 fragNormalized;

void main() {

	vec4 color = texture(uniformTex, fragUV);
	color *= tint;
	
	fragColor      = color;
	fragPosition   = fragPos;
	fragNormalized = fragNormal;
	
	//float unique = (gl_FragCoord.x + 1) * 2 + (gl_FragCoord.y + 1) * 4;
	//if(int(ftime * unique) % 2 == 0)
	//	fragColor = vec4(gl_FragColor.rgb, 0);
}