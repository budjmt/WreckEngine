#version 400

uniform vec4 tint;

uniform sampler2D uniformTex;
in vec3 fragPos;
in vec2 fragTexUV;
in vec3 fragNormal;

in vec3 camDir;

in float ftime;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 brightColor;

void main() {
	vec3 lightPos = vec3(3,2,0);
	vec3 lightDir = normalize(fragPos - lightPos);
	
	float lamb = max(dot(fragNormal, -lightDir), 0);
	float spec = 0.;
	if(lamb > 0) {
		vec3 viewDir = camDir;
		vec3 halfDir = normalize(lightDir + viewDir);
		float specAngle = max(dot(fragNormal, halfDir), 0);
		float specularity = 16.;
		spec = pow(specAngle, specularity);
	}

	vec4 color = texture(uniformTex, fragTexUV);
	color *= tint;
	//color = vec4(1,0.5,0.,1.);
	
	vec4  ambColor = vec4(0.6,0.6,1.,1.);
	float ambPower = 1.1;
	vec4  ambient  = ambPower * ambColor;
	
	vec4  lambColor = vec4(1.,1.,0.,1.);
	float lambPower = 0.8;
	vec4  diffuse   = lamb * lambPower * lambColor;
	
	vec4  specColor = vec4(1.);
	float specPower = 0.6;
	vec4  specular  = spec * specPower * specColor;
	
	vec4  fresColor = vec4(1.);
	float fresPower = 0.08;
	vec4  fresnel   = -dot(camDir, fragNormal) * fresPower * fresColor;
	
	fragColor = color * vec4((ambient + diffuse + specular + fresnel).rgb, 1.);
	
	float brightness = dot(fragColor.rgb, vec3(0.25, 0.75, 0.08));
	if (brightness > 1.)
		brightColor = fragColor;
	
	//float unique = (gl_FragCoord.x + 1) * 2 + (gl_FragCoord.y + 1) * 4;
	//if(int(ftime * unique) % 2 == 0)
	//	fragColor = vec4(gl_FragColor.rgb, 0);
}