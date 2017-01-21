#version 450

layout (location = 0) in vec3 vecPos;
layout (location = 1) in vec2 vecUV;
layout (location = 2) in vec3 vecNormal;
out vec4 fragPos;
out vec2 fragUV;
out vec3 fragNormal;

layout (location = 0) uniform float time;
layout (location = 1) uniform mat4 worldMatrix;
layout (location = 5) uniform mat4 iTworldMatrix;
layout (location = 9) uniform mat4 cameraMatrix;

out float ftime;

void main() {
	vec4 worldPos = worldMatrix * vec4(vecPos, 1);
	gl_Position = cameraMatrix * worldPos;
	
	fragPos = worldPos;
	fragUV  = vecUV;
	
	// use inverse transpose of world mat to avoid uneven scale
	// use 0 as fourth component to avoid translation
	fragNormal = normalize((iTworldMatrix * vec4(vecNormal, 0)).xyz);

	//ftime = time;
}