#version 400

in vec2 uv;

uniform sampler2D render;

out vec4 color;

void main() {
	color = texture(render, uv);
}