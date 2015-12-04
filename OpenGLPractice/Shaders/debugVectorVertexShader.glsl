#version 400

layout (location = 0) in vec3 position;

out vec4 vertColor;

void main() {
	gl_Position = vec4(position, 1);
	vec3 color = vec3(1,0,0);
	vertColor = vec4(color, 1);
}