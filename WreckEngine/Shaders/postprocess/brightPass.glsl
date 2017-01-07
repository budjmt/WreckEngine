#version 400

in vec2 uv;

uniform sampler2D render;

layout (location = 0) out vec4 fragColor;

void main() {
    vec4 color = texture(render, uv);
	float brightness = dot(fragColor.rgb, vec3(0.25, 0.75, 0.08));
	fragColor = brightness > 1 ? color : vec4(0);
}