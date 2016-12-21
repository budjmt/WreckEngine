#version 400

uniform vec4 tint;

uniform sampler2D uniformTex;
in vec3 fragPos;
in vec2 fragTexUV;
in vec3 fragNormal;

in float ftime;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 fragPosition;
layout (location = 2) out vec4 fragNormalized;

void main() {

	vec4 color = texture(uniformTex, fragTexUV);
	color *= tint;
	//color = vec4(1,0.5,0.,1.);
	
	fragColor = color;
	fragPosition = vec4(fragPos, 1.);
	fragNormalized = vec4(fragNormal * 0.5 + vec3(0.5), 1.);
	
	//float unique = (gl_FragCoord.x + 1) * 2 + (gl_FragCoord.y + 1) * 4;
	//if(int(ftime * unique) % 2 == 0)
	//	fragColor = vec4(gl_FragColor.rgb, 0);
}