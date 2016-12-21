#version 400

layout (location = 0) in vec3 vertPos;
layout (location = 1) in mat4 world;
// skip 4 for mat4
layout (location = 5) in vec3 position;
layout (location = 6) in flat int isOff;
layout (location = 7) in vec3 color;
layout (location = 8) in flat uint tag;
layout (location = 9) in vec2 falloff;

uniform mat4 camera;
uniform mat4 invCamera;

out mat4 invCam;
out vec2 uv;
out Point {
    vec3 position;
	vec3 color;
	vec2 falloff;
} light;

out vec3 camDir;

void main() {
	if(isOff == 0) {
		gl_Position = camera * world * vec4(vertPos, 1.);
		
		light.position = position;
		light.color    = color;
		light.falloff  = falloff;
		
		invCam = invCamera;
		uv = gl_Position.xy;
		camDir = camera[3].xyz;
	}
	else {
		gl_Position = vec4(vec2(-2.), vec2(1.)); // causes the vertex to be clipped
	}
}