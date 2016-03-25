#version 400

layout (location = 0) in vec3 vecPos;
layout (location = 1) in vec2 vecTexUV;
layout (location = 2) in vec3 vecNormal;
out vec3 fragPos;
out vec2 fragTexUV;
out vec3 fragNormal;

uniform float time;
uniform mat4 worldMatrix;
uniform mat4 iTworldMatrix;
uniform mat4 cameraMatrix;

void main() {
	vec4 worldPos = worldMatrix * vec4(vecPos, 1);
	gl_Position = cameraMatrix * worldPos;
	//vec3 color = vec3(0,1,0);
	fragPos = worldPos.xyz;
	fragTexUV = vecTexUV;
	//use inverse transpose of world mat to avoid uneven scale
	//use fourth component as 0 to avoid translation
	fragNormal = normalize(iTworldMatrix * vec4(vecNormal, 0)).xyz;
}