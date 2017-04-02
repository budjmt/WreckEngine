#version 450

in vec2 uv;

uniform sampler2D color;
uniform sampler2D diffuse;
uniform sampler2D specular;

uniform vec3 ambient;
uniform float exposure = 1;

layout (location = 0) out vec4 fragColor;

vec4 tonemap(in vec4 color) {
    const vec3 inv_gamma = vec3(1 / 2.2);
    vec3 hdr = color.rgb;
    hdr = vec3(1) - exp(-hdr * exposure);
    hdr = pow(hdr, inv_gamma);
    return vec4(hdr, color.a);
}

void main() {
	fragColor = texture(color, uv) * (texture(diffuse, uv) + vec4(ambient,0)) + texture(specular, uv);
	//fragColor = texture(color, uv);
    //fragColor = tonemap(fragColor);
}