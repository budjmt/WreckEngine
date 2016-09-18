#version 400

uniform vec4 tint;

uniform sampler2D uniformTex;
in vec3 fragPos;
in vec2 fragTexUV;
in vec3 fragNormal;

void main() {
	vec3 lightPos = vec3(3,2,0);
	vec3 lightDir = fragPos - lightPos;
	float dist = length(lightDir);
	lightDir /= dist;
	dist *= dist;
	//vec3 lightDir = normalize(vec3(1,-1,1));
	
	float lamb = max(dot(fragNormal, -lightDir),0);
	float spec = 0.;
	if(lamb > 0) {
		vec3 viewDir = normalize(-fragPos);
		vec3 halfDir = normalize(lightDir + viewDir);
		float specAngle = max(dot(fragNormal, halfDir),0);
		float specularity = 16.;
		spec = pow(specAngle,specularity);
	}
	//light /= dist;

	vec4 color = texture(uniformTex, fragTexUV);
	color *= tint;
	//color = vec4(1,0.5,0.,1.);
	
	float ambPower  = 0.04;
	float lambPower = 0.8;
	float specPower = 0.6;
	
	vec4 ambient  = ambPower * vec4(0.5,0.1,1.,1.);
	vec4 diffuse  = lamb * lambPower * vec4(1.,1.,0.,1.);
	vec4 specular = spec * specPower * vec4(1.,1.,1.,1.);
	
	gl_FragColor  = color + ambient + diffuse + specular;
}