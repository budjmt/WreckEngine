#version 400

in vec2 uv;

uniform sampler2D text;
uniform vec4 textColor;

layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = vec4(textColor.rgb, textColor.a * texture(text, uv).r);
}