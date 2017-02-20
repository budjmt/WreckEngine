#version 450

layout(location = 0) in vec3 Position;
out vec3 vPosition;

uniform mat4 worldMatrix;

void main()
{
    vPosition = vec3(worldMatrix * vec4(Position, 1.0));
}
