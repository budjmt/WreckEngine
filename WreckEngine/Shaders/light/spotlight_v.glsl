#version 400

layout (location = 0) in vec3 vertPos;
layout (location = 1) in mat4 world;
// skip 4 for mat4
layout (location = 5)  in vec3 position;
layout (location = 6)  in int  isOff;
layout (location = 7)  in vec3 direction;
layout (location = 8)  in uint tag;
layout (location = 9)  in vec2 falloffRad;
layout (location = 10) in vec2 falloffLen;
layout (location = 11) in vec3 color;

uniform mat4 camera;

out mat4 camMat;
out Spotlight {
    vec3 position;
    vec3 direction;
    vec3 color;
    vec2 falloffRad;
    vec2 falloffLen;
} light;

void main() {
	if(isOff == 0) {
		gl_Position = camera * world * vec4(vertPos, 1.);
		
		light.position   = position;
		light.direction  = direction;
		light.color      = color;
		light.falloffRad = falloffRad;
		light.falloffLen = falloffLen;
		
		camMat = camera;
	}
	else {
		gl_Position = vec4(vec2(-2.), vec2(1.)); // causes the vertex to be clipped
	}
}