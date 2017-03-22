#version 450

out vec4 FragColor;

in vec3 teNormal;

layout (binding = 0) uniform samplerCube NoiseTex;
layout (binding = 1) uniform samplerCube NormalTex;

void main()
{
#if 1
	vec4 color = texture(NormalTex, teNormal);
	color.a = 1.0;
#else
    vec4 color = texture(NoiseTex, teNormal);
#endif
    FragColor = color;
    //FragColor = vec4(color.rgb * teNormal, 1.0);
    //FragColor = vec4(teNormal * 0.5 + 0.5, 1.0);
    //FragColor = vec4(1,0,0,1);
}
