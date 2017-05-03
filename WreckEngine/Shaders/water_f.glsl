#version 450

layout(location = 0) out vec4 fragColor;

in vec3 fragNormal;
in vec2 fragUV;
in vec3 fragPosition;

layout(binding = 0) uniform sampler2D normalMap;
layout(location = 1) uniform vec3 camPos;
layout(location = 5) uniform float time;
layout(location = 6) uniform vec3 sunDir;
layout(location = 7) uniform vec3 viewDir;

const vec3 waterColor = { 0.2, 0.2, 0.8 };
const float waveSpeed = 0.25;
const float specularPower = 512.0;

void main()
{
    vec2 uv = fragUV * 4.0;
    uv.y -= time * waveSpeed;

    vec3 N = fragNormal;
    vec4 sampledNormal = texture(normalMap, uv);
    vec3 normal = sampledNormal.rgb * 2.0 - 1.0;

    vec3 viewDir = normalize(camPos - fragPosition);
    vec3 specularVector = reflect(sunDir, normal);
    float specular = dot(normalize(specularVector), viewDir);
    specular = pow(specular, specularPower);

    vec3 color = waterColor * (specular + 1.0);
    fragColor = vec4(color, 0.9);
}
