#version 400

layout (location = 0) in vec4 vertex;
out vec2 uv;

uniform mat4 camera; // generally just projection

void main() {
	gl_Position = camera * vec4(vertex.xy, 0.01, 1.0);
	uv = vertex.zw;
}