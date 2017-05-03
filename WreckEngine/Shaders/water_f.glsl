#version 450

layout(location = 0) out vec4 fragColor;

in vec3 fragNormal;
in vec2 fragUV;

layout(binding = 0) uniform sampler2D normalMap;
layout(location = 5) uniform float time;

void main()
{
    vec3 N = fragNormal;
    vec4 sampledNormal = texture(normalMap, fragUV);
    vec3 color = sampledNormal.rgb * 2.0 - 1.0;

    color = vec3(fragUV, 0);
    fragColor = vec4(color, 1);
}
