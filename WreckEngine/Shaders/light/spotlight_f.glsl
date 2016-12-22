#version 400

in Spotlight {
    vec3 position;
	vec2 falloffRad;
	vec2 falloffLen;
	vec3 direction;
	vec3 color;
} light;

uniform vec3 camPos;
uniform vec2 resolution;

// object render targets
uniform sampler2D gPosition;
uniform sampler2D gNormal;

// light accumulation targets
layout(location = 3) out vec4 diffuseColor;
layout(location = 4) out vec4 specularColor;

void main() {
	vec2 uv = gl_FragCoord.xy / resolution;
	
	vec3 normal  = texture(gNormal, uv).rgb * 2. - 1.;
    if(normal == vec3(0)) discard;
    
	vec3 fragPos = texture(gPosition, uv).rgb;
	
	vec3  toLight  = light.position - fragPos;
	float distSq   = dot(toLight, toLight);
	float dist     = sqrt(distSq);
	vec3  lightDir = normalize(toLight);
	
	vec3 viewDir  = normalize(camPos - fragPos);
	vec3 hDir     = normalize(lightDir + viewDir);
	
	float distDir  = -dot(toLight, light.direction);
	float lenFac   = distDir / light.falloffLen.y;
	vec2  discRad  = lenFac * light.falloffRad;
	float radLen   = sqrt(distSq - distDir * distDir);
	
	float aRad     = max(radLen - discRad.x, 0.);
	float attenRad = 1. - min(aRad / (discRad.y - discRad.x), 1.);
	
	float aLen     = max(dist - light.falloffLen.x, 0.);
	float attenLen = 1. -  min(aLen / (light.falloffLen.y - light.falloffLen.x), 1.);
	
	float atten   = attenLen + attenRad - 1.;
	
	vec3 diffuse  = max(dot(normal, lightDir), 0.) * atten * light.color;
	
	float spec    = pow(max(dot(normal, hDir), 0.), 16.);
	vec3 specular = spec * atten * light.color;
	
	diffuseColor  = vec4(diffuse, 1.);
	specularColor = vec4(specular, 1.);
}