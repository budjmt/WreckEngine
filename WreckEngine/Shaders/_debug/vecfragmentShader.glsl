#version 400

in vec4 vertColor;

layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = vertColor;
}