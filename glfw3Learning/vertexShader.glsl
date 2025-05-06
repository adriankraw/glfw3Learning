#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

uniform mat4 transform;
uniform mat4 view;
uniform vec3 transposition;

out vec3 vertexColor;
out vec3 normal;
out vec4 fragPos;

void main()
{
    vec4 position = view * vec4(aPos.x, aPos.y, aPos.z, 1.0F);
    position = transform * vec4(position.x, position.y, position.z, 1.0F);
    position.x += transposition.x;
    position.y += transposition.y;
    position.z += transposition.z;
    fragPos = position;
    gl_Position = fragPos;
    vertexColor = aColor;
    normal = aNormal;
}
