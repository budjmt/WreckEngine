#version 400

in vec2 uv;
//out vec4 color;

uniform sampler2D text;
uniform vec4 textColor;

void main() {
	gl_FragColor = vec4(textColor.rgb, textColor.a * texture(text, uv).r);
}