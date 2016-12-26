#version 400

in mat4 camMat;
in Spotlight {
    vec3 position;
    vec3 direction;
    vec3 color;
    vec2 falloffRad;
    vec2 falloffLen;
} light;

uniform vec3 camPos;
uniform vec2 resolution;

// object render targets
uniform sampler2D gPosition;
uniform sampler2D gNormal;

// light accumulation targets
layout (location = 4) out vec4 diffuseColor;
layout (location = 5) out vec4 specularColor;

void main() {
	vec2 uv = gl_FragCoord.xy / resolution;

	vec3 normal  = texture(gNormal, uv).rgb;
    if(normal == vec3(0)) discard;
    
	vec4 pos      = texture(gPosition, uv);
	vec3 worldPos = pos.rgb;
	vec3 fragPos  = (camMat * pos).rgb;
	
	vec3  toLight  = light.position - worldPos;
	vec3  lightDir = normalize(toLight);
	
	float dist     = length(toLight);
	float distSq   = dist * dist;
	float distDir  = max(-dot(toLight, light.direction), 0);
	
	vec3 viewDir  = normalize(-fragPos);
	vec3 hDir     = normalize(lightDir + viewDir);
	
	float lenFact  = distDir / light.falloffLen.y;
	vec2  discRad  = lenFact * light.falloffRad;
	float radLen   = sqrt(distSq - distDir * distDir);
	
	float aRad     = max(radLen - discRad.x, 0.);
	float attenRad = 1. - min(aRad / (discRad.y - discRad.x), 1.);
	
	float aLen     = max(distDir - light.falloffLen.x, 0.);
	float attenLen = 1. -  min(aLen / (light.falloffLen.y - light.falloffLen.x), 1.);
	
	float atten   = max(attenLen + attenRad - 1., 0);
	
	vec3 diffuse  = max(dot(normal, lightDir), 0.) * atten * light.color;
	
	float spec    = pow(max(dot(normal, hDir), 0.), 16.);
	vec3 specular = spec * atten * light.color;
	
	diffuseColor  = vec4(diffuse, atten);
	specularColor = vec4(specular, atten);
}