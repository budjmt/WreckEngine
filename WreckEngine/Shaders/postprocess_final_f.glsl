#version 400

in vec2 uv;

out vec3 color;

uniform sampler2D render;

void main() {
	color = texture(render, uv).xyz;
}