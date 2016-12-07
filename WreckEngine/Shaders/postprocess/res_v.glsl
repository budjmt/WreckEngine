#version 400

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 inUV;

out vec2 uv;

void main() {
	gl_Position = vec4(pos, 0, 1);
	uv = inUV;
}