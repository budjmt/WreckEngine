#version 400

uniform vec4 tint;
in vec4 vertColor;

void main() {
	gl_FragColor = tint * vertColor;
}