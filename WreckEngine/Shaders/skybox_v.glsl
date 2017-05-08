#version 450

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec2 vertUv;
layout(location = 2) in vec3 vertNorm;

out vec3 fragPosition;
out vec3 fragNormal;

layout(location = 0) uniform mat4 viewProjection;

void main() {
    vec4 worldPosition = viewProjection * vec4(vertPos, 1);
    fragPosition = worldPosition.xyz / worldPosition.w;
    fragNormal = normalize(fragPosition);

    gl_Position = worldPosition;
}
