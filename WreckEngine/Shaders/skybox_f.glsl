#version 450

layout(location = 0) out vec4 fragColor;

in vec3 fragPosition;
//in vec3 fragNormal;

layout (location = 1) uniform vec2 atmosRadius;
layout (location = 2) uniform float camHeight;

layout(binding = 0) uniform samplerCube spaceSky;
layout(binding = 1) uniform samplerCube planetSky;

void main() {
    vec4 sky = texture(planetSky, fragPosition);
    vec4 space = texture(spaceSky, fragPosition);
    
    float heightProp = (camHeight - atmosRadius.x) / (atmosRadius.y - atmosRadius.x);
    heightProp = heightProp * 1.5 - 0.5; // move transition start from 0 to 1/3, leave top at 1
    
    fragColor = mix(sky, space, clamp(heightProp, 0, 1));
}
