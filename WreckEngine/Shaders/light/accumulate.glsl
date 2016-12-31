#version 450

in vec2 uv;

uniform sampler2D color;
uniform sampler2D diffuse;
uniform sampler2D specular;

uniform vec3 ambient;

layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = texture(color, uv) * (texture(diffuse, uv) + vec4(ambient,0)) + texture(specular, uv);
	//fragColor = texture(specular, uv);
}