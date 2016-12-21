#version 400

layout (location = 0) in vec2 vertPos;
layout (location = 1) in vec2 vertUv;

layout (location = 2) in vec3 direction;
layout (location = 3) in flat int isOff;
layout (location = 4) in vec3 color;
layout (location = 5) in flat uint tag;

uniform mat4 camera;
uniform mat4 invCamera;

out vec2 uv;
out Directional {
    vec3 direction;
	vec3 color;
} light;

out vec3 camDir;

void main() {
	if(isOff == 0) {
		gl_Position = vec4(vertPos, 0, 1);
		
		light.direction = direction;
		light.color     = color;
		
		uv = vertUv;
		camDir = camera[3].xyz;
	}
	else {
		gl_Position = vec4(vec2(-2.), vec2(1.)); // causes the vertex to be clipped
	}
}