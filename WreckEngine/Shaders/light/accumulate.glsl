#version 400

in vec2 uv;

uniform sampler2D color;
uniform sampler2D diffuse;
uniform sampler2D specular;

uniform vec3 ambient;

layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = texture(color, uv) * (texture(diffuse, uv) + ambient) + texture(specular, uv);
	//fragColor = texture(diffuse, uv);
}