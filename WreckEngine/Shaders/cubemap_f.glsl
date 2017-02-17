#version 450

out vec4 FragColor;

in vec3 teNormal;

layout (binding = 0) uniform samplerCube Tex;

void main()
{
    vec4 color = texture(Tex, teNormal);
    FragColor = color;
    //FragColor = vec4(color.rgb * teNormal, 1.0);
    //FragColor = vec4(teNormal * 0.5 + 0.5, 1.0);
}
