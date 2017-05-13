#version 450

layout (location = 0) out vec4 fragPosition;
layout (location = 1) out vec4 fragNormalized;
layout (location = 2) out vec4 diffColor;
layout (location = 3) out vec4 specColor;

in vec3 position;
in vec3 fragNormal;
in vec3 tessCoord;

layout (binding = 1) uniform samplerCube heightMap;
layout (binding = 2) uniform samplerCube normalMap;

layout (binding = 3) uniform sampler2D grassTex;
layout (binding = 4) uniform sampler2D mountainTex;
layout (binding = 5) uniform sampler2D snowTex;

vec3 getElevationColor(in float h) {
    vec3 blend = abs(fragNormal);
    blend = normalize(max(blend, 0.00001));
    blend /= vec3(blend.x + blend.y + blend.z);
    
    if(h > 0.650) {
        vec3 x = texture(snowTex, position.yz).rgb;
        vec3 y = texture(snowTex, position.xz).rgb;
        vec3 z = texture(snowTex, position.xy).rgb;
        //return vec3(1);
        return x * blend.x + y * blend.y + z * blend.z;
    }
    if(h > 0.275) {
        vec3 x = texture(mountainTex, position.yz).rgb;
        vec3 y = texture(mountainTex, position.xz).rgb;
        vec3 z = texture(mountainTex, position.xy).rgb;
        //return vec3(0.45, 0.25, 0);
        return x * blend.x + y * blend.y + z * blend.z;
    }
    if(h > 0.000) {
        vec3 x = texture(grassTex, position.yz).rgb;
        vec3 y = texture(grassTex, position.xz).rgb;
        vec3 z = texture(grassTex, position.xy).rgb;
        //return vec3(0, 1, 0);
        return x * blend.x + y * blend.y + z * blend.z;
    }

    return vec3(0, 0.2, 1) * ((h + 1) / 1.05);
}

float height(in vec3 coord) {
    return texture(heightMap, coord).r;
}

void main()
{
    vec3 N = texture(normalMap, fragNormal).rgb;
    vec3 No = N;
    //N = vec3(0);

    vec3 elevationColor = getElevationColor(height(fragNormal));
    //vec3 color = fragNormal;
    //vec3 color = vec3(height(fragNormal));
    vec3 color = elevationColor;
    //vec3 color = N;
    //vec3 color = No;
    //vec3 color = vec3(1);

    fragPosition = vec4(position, 1);
    fragNormalized = vec4(N, 1);
    diffColor = vec4(color, 1);
    specColor = vec4(color / 4, 1);
}
