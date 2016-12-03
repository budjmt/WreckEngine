#version 400

in vec2 uv;

uniform sampler2D render;
uniform sampler2D brightBlur;

layout (location = 0) out vec4 fragColor;

void main() {
    fragColor = texture(render, uv) + texture(brightBlur, uv);
}