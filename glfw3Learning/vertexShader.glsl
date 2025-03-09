#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

uniform mat4 transform;

out vec3 Color;
void main()
{
    gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0F);
    float z = gl_Position.z;
    Color = vec3(z) * aColor;
}
