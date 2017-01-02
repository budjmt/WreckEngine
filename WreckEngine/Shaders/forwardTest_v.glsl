#version 400

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertUv;
layout (location = 2) in vec3 vertNorm;

out vec4 worldPos;
out vec3 normal;
out vec2 uv;

uniform mat4 worldMatrix;
uniform mat4 iTworldMatrix;
uniform mat4 cameraMatrix;

void main() {
	worldPos = worldMatrix * vec4(vertPos, 1);
	gl_Position = cameraMatrix * worldPos;
	
	normal = normalize((iTworldMatrix * vec4(vertNorm, 0)).xyz);
	
	uv = vertUv;
}