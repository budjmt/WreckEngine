#version 400

layout (location = 0) in vec3 position;

out vec4 vertColor;

uniform mat4 cameraMatrix;

/*mat4 worldMatrix = mat4(
	vec4(1,0,0,0),
	vec4(0,1,0,0),
	vec4(0,0,1,0),
	vec4(0,0,0,1)
);*/

void main() {
	gl_Position = cameraMatrix * vec4(position, 1);
	vec3 color = vec3(0.7,1,0);
	vertColor = vec4(color, 1);
}