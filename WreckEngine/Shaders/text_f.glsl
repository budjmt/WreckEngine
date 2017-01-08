#version 400

in vec2 fUV;
flat in uint fColor;

uniform sampler2D text;

layout (location = 0) out vec4 fragColor;

vec4 unpackColor() {
    uint r = (fColor >> 24) & 255;
    uint g = (fColor >> 16) & 255;
    uint b = (fColor >>  8) & 255;
    uint a = (fColor >>  0) & 255;
    return vec4(r, g, b, a) / 255.0;
}

void main() {
    vec4 charColor = unpackColor();
    float alpha = texture(text, fUV).r;
    fragColor = vec4(charColor.rgb, charColor.a * alpha);
}
