#version 400

uniform vec4 tint;
in vec4 vertColor;

layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = tint * vertColor;
}