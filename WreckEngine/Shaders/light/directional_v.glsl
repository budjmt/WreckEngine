#version 450

layout (location = 0) in vec2 vertPos;
layout (location = 1) in vec2 vertUv;
layout (location = 2) in mat4 lightWorldViewProj;

layout (location = 6)  in vec3 direction;
layout (location = 7)  in int  isOff;
layout (location = 8)  in vec3 color;
layout (location = 9)  in uint tag;
layout (location = 10) in int  castsShadow;

uniform mat4 camera;

out vec2 uv;
out Directional {
    vec3 direction;
	vec3 color;
} light;

void main() {
	if(isOff == 0) {
		gl_Position = vec4(vertPos, 0, 1);
		
		light.direction = direction;
		light.color     = color;
		
		uv = vertUv;
	}
	else {
		gl_Position = vec4(vec2(-2.), vec2(1.)); // causes the vertex to be clipped
	}
}