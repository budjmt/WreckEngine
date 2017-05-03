#version 450

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec2 vertUv;
layout(location = 2) in vec3 vertNorm;

out vec3 vPosition;
out vec2 vUV;

layout(location = 0) uniform mat4 worldMatrix;

void main() {
  vPosition = (worldMatrix * vec4(vertPos, 1)).xyz;
  vUV = vertUv;
}
