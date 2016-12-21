#version 400

in mat4 invCam;
in vec2 uv;
in Spotlight {
    vec3 position;
	vec2 falloffRad;
	vec2 falloffLen;
	vec3 direction;
	vec3 color;
} light;

in vec3 camDir;

// object render targets
uniform sampler2D gPosition;
uniform sampler2D gNormal;

// light accumulation targets
layout(location = 3) out vec4 diffuseColor;
layout(location = 4) out vec4 specularColor;

void main() {
    vec4 fragPos = texture(gPosition, uv);
	vec3 pos     = (invCam * fragPos).rgb;
	
	vec3 normal  = (texture(gNormal, uv).rgb - vec3(0.5)) * 2.;
	
	vec3  toLight  = light.position - pos;
	float distSq   = dot(toLight, toLight);
	float dist     = sqrt(distSq);
	vec3  lightDir = normalize(toLight);
	
	vec3 viewDir  = camDir;
	vec3 hDir     = normalize(lightDir + viewDir);
	
	float distDir  = -dot(toLight, light.direction);
	float lenFac   = distDir / light.falloffLen.y;
	vec2  discRad  = lenFac * light.falloffRad;
	float radLen   = sqrt(distSq - distDir * distDir);
	
	float aRad     = max(radLen - discRad.x, 0.);
	float attenRad = 1. - aRad / (discRad.y - discRad.x);
	
	float aLen     = max(dist - light.falloffLen.x, 0.);
	float attenLen = 1. -  aLen / (light.falloffLen.y - light.falloffLen.x);
	
	float atten   = max(attenLen + attenRad - 1., 0.);
	
	vec3 diffuse  = max(dot(normal, lightDir), 0.) * atten * light.color;
	
	float spec    = pow(max(dot(normal, hDir), 0.), 16.);
	vec3 specular = spec * atten * light.color;
	
	diffuseColor  = vec4(diffuse, 1.);
	specularColor = vec4(specular, 1.);
}