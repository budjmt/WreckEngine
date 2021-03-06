#version 450

layout(location = 0) out vec4 fragColor;

in vec3 fragNormal;
in vec2 fragUV;
in vec3 fragPosition;

layout(binding = 1) uniform sampler2D normalMap;
layout(binding = 2) uniform samplerCube skyBox;
layout(location = 1) uniform vec3 camPos;
layout(location = 5) uniform float time;
layout(location = 6) uniform vec3 sunPos;

const vec3 waterColor = vec3(0.0, 0.1, 0.5);
const float waveSpeed = 0.0625;
const float specularPower = 16.0;
const float reflectAmount = 0.15;

void main()
{
    vec2 uv = fragUV * 8.0;
    uv.y -= time * waveSpeed;

    vec3 N = fragNormal;
    vec4 sampledNormal = texture(normalMap, uv);
    vec3 normal = sampledNormal.rgb * 2.0 - 1.0;

    vec3 sunDir = normalize(sunPos - fragPosition);
    vec3 viewDir = normalize(camPos - fragPosition);
    vec3 specularVector = reflect(sunDir, normal);
    float specular = dot(normalize(specularVector), viewDir);
    specular = pow(specular, specularPower);
    //specular = max(specular, 0);

    vec3 skyColor = texture(skyBox, normalize(N)).rgb;
    vec3 color = max(waterColor, waterColor * (specular + 1.0));

    vec3 finalColor = mix(color, skyColor, reflectAmount);
    fragColor = vec4(color, 0.9);
}
