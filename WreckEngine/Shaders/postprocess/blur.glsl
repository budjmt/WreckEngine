#version 400

in vec2 uv;

uniform bool horizontal;
uniform sampler2D render;

layout (location = 0) out vec4 fragColor;

float weights[5] = float[] ( 0.25, 0.2, 0.125, 0.05, 0.02 );

vec3 blur(in vec3 result, in vec2 offset) {
	result *= weights[0];
	for(int i = 1; i < 5; ++i) {
		result += texture(render, uv + offset * i).rgb * weights[i];
		result += texture(render, uv - offset * i).rgb * weights[i];
	}
	return result;
}

void main() {
	vec2 offset = 1. / textureSize(render, 0);
	if(horizontal) offset = vec2(offset.x, 0.);
	else           offset = vec2(0., offset.y);
	
	vec4 tex = texture(render, uv);
    fragColor = vec4(blur(tex.rgb, offset).rgb, tex.a);
}