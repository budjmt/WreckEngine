#version 400

layout (location = 0) in vec2 inUV;

out vec2 uv;

void main() {
	uv = inUV;
	gl_Position = vec4(uv * 2.f - 1.f, 0.f, 1.f);
}