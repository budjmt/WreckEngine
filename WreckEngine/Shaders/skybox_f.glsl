#version 450

layout(location = 0) out vec4 fragColor;

in vec3 fragPosition;
//in vec3 fragNormal;

layout(binding = 0) uniform samplerCube spaceSky;
layout(binding = 1) uniform samplerCube planetSky;

void main() {
    vec3 color = vec3(1, 0, 0);

    fragColor = vec4(color, 1.0);
    fragColor = texture(planetSky, fragPosition);
}
