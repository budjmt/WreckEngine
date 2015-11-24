#version 400

layout (location = 0) in vec3 vecPos;
layout (location = 1) in vec2 vecTexUV;
layout (location = 2) in vec3 vecNormal;
out vec3 fragPos;
out vec2 fragTexUV;
out vec3 fragNormal;

uniform mat4 worldMatrix;
uniform mat4 cameraMatrix;

void main() {
	gl_Position = cameraMatrix * worldMatrix * vec4(vecPos, 1);
	//vec3 color = vec3(0,1,0);
	fragPos = gl_Position.xyz;
	fragTexUV = vecTexUV;
	//fragNormal = normalize(worldMatrix * vec4(vecNormal, 1)).xyz;
	fragNormal = (vecNormal + vec3(1,1,1)) * 0.5;
}