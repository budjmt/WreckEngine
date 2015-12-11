#version 400

layout (location = 0) in vec3 position;

out vec4 vertColor;

uniform mat4 worldMatrix;
uniform mat4 cameraMatrix;

void main() {
	gl_Position = cameraMatrix * worldMatrix * vec4(position, 1);
	vec3 color = vec3(0.8,0.7,1);
	vertColor = vec4(color, 0.3);
}