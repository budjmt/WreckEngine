#version 400

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
out vec4 vertColor;

uniform mat4 worldMatrix;

void main() {
	gl_Position = worldMatrix * vec4(position, 1);
	//vec3 color = vec3(0,1,0);
	vertColor = vec4(color, 1);
}