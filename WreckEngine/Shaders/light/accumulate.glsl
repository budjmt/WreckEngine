#version 450

in vec2 uv;

layout (binding = 0) uniform sampler2D diffuseColor;
layout (binding = 1) uniform sampler2D specularColor;
layout (binding = 2) uniform sampler2D diffuseLight;
layout (binding = 3) uniform sampler2D specularLight;

uniform vec3 ambient;
uniform float exposure = 1;

layout (location = 0) out vec4 fragColor;

vec4 tonemap(in vec4 color) {
    const vec3 inv_gamma = vec3(1 / 2.2);
    vec3 hdr = color.rgb;
    hdr = vec3(1) - exp(hdr * -exposure);
    //hdr = pow(hdr, inv_gamma);
    return vec4(hdr, color.a);
}

void main() {
    vec4 diffuseLightColor = texture(diffuseLight, uv);
    vec3 addDiffuse = (diffuseLightColor.a == 0) ? vec3(1) : ambient;
    fragColor = texture(diffuseColor, uv) * (diffuseLightColor + vec4(addDiffuse, 0)) 
                + texture(specularColor, uv) * texture(specularLight, uv);
    fragColor.a = 1;
    //fragColor = texture(diffuseColor, uv);
    //fragColor = tonemap(fragColor);
}