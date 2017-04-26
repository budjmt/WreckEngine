#version 450

in vec2 uv;

uniform sampler2D render;

layout (location = 0) uniform float exposure = 2;

layout (location = 0) out vec4 color;

vec4 tonemap(in vec4 color) {
    //const vec3 inv_gamma = vec3(1 / 2.2);
    vec3 hdr = color.rgb;
    hdr = vec3(1) - exp(hdr * -exposure);
    //hdr /= hdr + vec3(1);
    //hdr = pow(hdr, inv_gamma);
    return vec4(hdr, color.a);
}

void main() {
    color = tonemap(texture(render, uv));
}