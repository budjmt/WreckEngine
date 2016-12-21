#version 400

layout (location = 0) in vec3 vertPos;
layout (location = 1) in mat4 world;
// skip 4 for mat4
layout (location = 5)  in vec3 position;
layout (location = 6)  in flat int isOff;
layout (location = 7)  in vec2 falloffRad;
layout (location = 8)  in vec2 falloffLen;
layout (location = 9)  in vec3 direction;
layout (location = 10) in flat uint tag;
layout (location = 11) in vec3 color;

uniform mat4 camera;
uniform mat4 invCamera;

out mat4 invCam;
out vec2 uv;
out Spotlight {
    vec3 position;
	vec2 falloffRad;
	vec2 falloffLen;
	vec3 direction;
	vec3 color;
} light;

out vec3 camDir;

void main() {
	if(isOff == 0) {
		gl_Position = camera * world * vec4(vertPos, 1.);
		
		light.position   = position;
		light.falloffRad = falloffRad;
		light.falloffLen = falloffLen;
		light.direction  = direction;
		light.color      = color;
		
		invCam = invCamera;
		uv = gl_Position.xy;
		camDir = camera[3].xyz;
	}
	else {
		gl_Position = vec4(vec2(-2.), vec2(1.)); // causes the vertex to be clipped
	}
}