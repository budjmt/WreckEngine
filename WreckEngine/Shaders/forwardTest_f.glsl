#version 450

struct Point {
    vec4 position;
	vec4 color;
	vec4 falloff;
};

struct Spotlight {
    vec4 position;
	vec4 direction;
	vec2 falloffRad;
	vec2 falloffLen;
	vec4 color;
};

struct Directional {
    vec4 direction;
	vec4 color;
};

layout (std140) uniform PointBlock {
    uvec4 size;
    Point list[500];
} points;

layout (std140) uniform SpotlightBlock {
    uvec4 size;
    Spotlight list[100];
} spotlights;

layout (std140) uniform DirectionalBlock {
    uvec4 size;
    Directional list[50];
} directionals;

layout (location = 0) out vec4 fragColor;

struct Data {
    vec3 worldPos;
	vec3 viewDir;
	vec3 normal;
};

vec4 calcDiffuse(in vec3 lightDir, in vec3 normal, in vec3 color, in float atten) {
    return vec4(max(dot(normal, lightDir), 0.) * atten * color, 0);
}

vec4 calcSpecular(in vec3 halfDir, in vec3 normal, in vec3 color, in float atten) {
    return vec4(pow(max(dot(normal, halfDir), 0.), 16.) * atten * color, 0);
}

void calcPoint(in Data data, in Point light, inout vec4 diffuse, inout vec4 specular) {
    vec3 toLight  = light.position.xyz - data.worldPos;
	float dist    = length(toLight);
	vec3 lightDir = normalize(toLight);
	
	vec3 viewDir  = data.viewDir;
	vec3 halfDir  = normalize(lightDir + viewDir);
	
	float a       = max(dist - light.falloff.x, 0.);
	float atten   = 1. -  min(a / (light.falloff.y - light.falloff.x), 1.); // plain old linear attenuation
	
	diffuse  += calcDiffuse(lightDir, data.normal, light.color.rgb, atten);
	specular += calcSpecular(halfDir, data.normal, light.color.rgb, atten);
}

void calcSpotlight(in Data data, in Spotlight light, inout vec4 diffuse, inout vec4 specular) {
    vec3  toLight  = light.position.xyz - data.worldPos;
	vec3  lightDir = normalize(toLight);
	
	float dist     = length(toLight);
	float distSq   = dist * dist;
	float distDir  = max(-dot(toLight, light.direction.xyz), 0);
	
	vec3 viewDir  = data.viewDir;
	vec3 halfDir  = normalize(lightDir + viewDir);
	
	float lenFact  = distDir / light.falloffLen.y;
	vec2  discRad  = lenFact * light.falloffRad;
	float radLen   = sqrt(distSq - distDir * distDir);
	
	float aRad     = max(radLen - discRad.x, 0.);
	float attenRad = 1. - min(aRad / (discRad.y - discRad.x), 1.);
	
	float aLen     = max(distDir - light.falloffLen.x, 0.);
	float attenLen = 1. -  min(aLen / (light.falloffLen.y - light.falloffLen.x), 1.);
	
	float atten   = max(attenLen + attenRad - 1., 0);
	
	diffuse  += calcDiffuse(lightDir, data.normal, light.color.rgb, atten);
	specular += calcSpecular(halfDir, data.normal, light.color.rgb, atten);
}

void calcDirectional(in Data data, in Directional light, inout vec4 diffuse, inout vec4 specular) {
    vec3 lightDir = -light.direction.xyz;
	
	vec3 viewDir  = data.viewDir;
	vec3 halfDir  = normalize(lightDir + viewDir);
	
	diffuse  += calcDiffuse(lightDir, data.normal, light.color.rgb, 1.);
	specular += calcSpecular(halfDir, data.normal, light.color.rgb, 1.);
}

void processLights(in Data data, inout vec4 diffuse, inout vec4 specular) {
    for(uint i = 0, size = points.size.x; i < size; ++i) {
	    Point p = points.list[i];
	    if(p.position.w == 0)
	        calcPoint(data, p, diffuse, specular);
	}
		
	for(uint i = 0, size = spotlights.size.x; i < size; ++i) {
	    Spotlight s = spotlights.list[i];
		if(s.position.w == 0)
	        calcSpotlight(data, s, diffuse, specular);
	}
		
	for(uint i = 0, size = directionals.size.x; i < size; ++i) {
	    Directional d = directionals.list[i];
		if(d.direction.w == 0)
			calcDirectional(data, d, diffuse, specular);
	}
}

//uniform sampler2D tex;
uniform vec4 tint;

uniform vec3 camPos;

in vec4 worldPos;
in vec3 normal;
in vec2 uv;

layout (location = 0) out vec4 color;

void main() {
	vec3 viewDir = normalize(camPos - worldPos.xyz);
	
    vec4 diffuse = vec4(0,0,0,1), specular = vec4(0,0,0,1);
	processLights(Data(worldPos.xyz, viewDir, normal), diffuse, specular);
	
	//color = vec4(spotlights.list[0].color.rgb, 1);
	color = tint;
	color = diffuse * color + specular;
}