#version 400

layout (location = 0) in vec3 vecPos;
layout (location = 1) in vec2 vecUV;
layout (location = 2) in vec3 vecNormal;
out vec4 fragPos;
out vec2 fragUV;
out vec4 fragNormal;

uniform float time;
uniform mat4 worldMatrix;
uniform mat4 iTworldMatrix;
uniform mat4 cameraMatrix;

out float ftime;

void main() {
	vec4 worldPos = worldMatrix * vec4(vecPos, 1);
	gl_Position = cameraMatrix * worldPos;
	
	fragPos = worldPos;
	fragUV  = vecUV;
	
	// use inverse transpose of world mat to avoid uneven scale
	// use 0 as fourth component to avoid translation
	fragNormal = normalize(iTworldMatrix * vec4(vecNormal, 0));

	//ftime = time;
}