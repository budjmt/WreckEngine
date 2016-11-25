#version 4.0

layout (location = 0) in vec2 uv;

out vec2 outUV;

void main() {
	outUV = uv;
	gl_Position = vec4(outUV * 2.f - 1.f, 0.f, 1.f);
}