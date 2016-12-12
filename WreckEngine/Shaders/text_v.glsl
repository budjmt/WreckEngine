#version 400

layout (location = 0) in vec2 vPosition;
layout (location = 1) in vec2 vUV;
layout (location = 2) in uint vColor;

out vec2 fUV;
flat out uint fColor;

uniform mat4 camera; // generally just projection
uniform vec2 offset;

void main() {
    fUV = vUV;
    fColor = vColor;
    gl_Position = camera * vec4(vPosition + offset, 0.01, 1.0);
}
