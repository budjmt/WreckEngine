#version 400

uniform vec4 tint;

uniform sampler2D uniformTex;
in vec3 fragPos;
in vec2 fragTexUV;
in vec3 fragNormal;

void main() {
	vec3 lightDir = vec3(1,1,-1);
	float lamb = max(dot(lightDir,fragNormal),0);
	vec3 viewDir = normalize(-fragPos);
	vec3 halfDir = normalize(lightDir + viewDir);
	float specAngle = max(dot(halfDir,fragNormal),0);
	float specular = pow(specAngle,16.0);
	float light = .3 + .6 * lamb + .3 * specular;

	vec4 color = texture(uniformTex, fragTexUV);
	color *= tint;
	//gl_FragColor = vec4(color.rgb * light,1);
	gl_FragColor = vec4(fragNormal,1);
	//gl_FragColor = vec4(fragTexUV,1,1);
}