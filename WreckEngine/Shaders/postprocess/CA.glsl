#version 400

in vec2 uv;

uniform sampler2D render;

layout (location = 0) out vec4 fragColor;

float offset = 0.003;
vec2 rOffset = vec2(offset, offset);
vec2 gOffset = vec2(offset / 2, 0);
vec2 bOffset = vec2(0, offset / 2);

void main() {
    float rValue = texture2D(render, uv - rOffset).r;  
    float gValue = texture2D(render, uv - gOffset).g;
    float bValue = texture2D(render, uv - bOffset).b;

    fragColor = vec4(rValue, gValue, bValue, 1);
}