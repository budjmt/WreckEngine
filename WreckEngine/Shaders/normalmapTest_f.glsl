#version 450

layout (location = 13) uniform vec4 tint = vec4(1);

layout (binding = 0) uniform sampler2D uniformTex;
layout (binding = 1) uniform sampler2D normalMap;
in vec4 fragPos;
in vec2 fragUV;
in mat3 TBN;

in float ftime;

layout (location = 0) out vec4 fragPosition;
layout (location = 1) out vec4 fragNormalized;
layout (location = 2) out vec4 diffColor;
layout (location = 3) out vec4 specColor;

void main() {

	vec4 color = texture(uniformTex, fragUV);
	color *= tint;
	//vec4 color = vec4(1,0,0,1);
	
    vec3 normal = normalize(texture(normalMap, fragUV).rgb * 2 - 1);
    normal = normalize(TBN * normal);
    
	fragPosition   = fragPos;
	fragNormalized = vec4(normal, 1);
    //fragNormalized = vec4(TBN[2], 1);
	diffColor      = color;
    specColor      = vec4(1);
	
	//float unique = (gl_FragCoord.x + 1) * 2 + (gl_FragCoord.y + 1) * 4;
	//if(int(ftime * unique) % 2 == 0)
	//	diffColor = vec4(gl_FragColor.rgb, 0);
}