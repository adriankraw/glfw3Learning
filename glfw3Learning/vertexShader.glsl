#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

uniform mat4 transform;
uniform mat4 view;
uniform vec3 transposition;
uniform vec3 transrotation;
uniform vec3 transscaling;

out vec3 vertexColor;
out vec3 normal;
out vec4 fragPos;

void main()
{
    vec4 position = vec4(aPos.x, aPos.y, aPos.z, 1.0F);

    position.x *= transscaling.x;
    position.y *= transscaling.y;
    position.z *= transscaling.z;

    mat3 rotationRoll = mat3(vec3(1.0f, 0.0f, 0.0f),
                             vec3(0.0f, cos(transrotation.x),-sin(transrotation.x)),
                             vec3(0.0f, sin(transrotation.x),cos(transrotation.x)));
    mat3 rotationPitch = mat3(vec3(cos(transrotation.y), 0.0f, sin(transrotation.y)),
                              vec3(0.0f, 1.0f, 0.0f),
                              vec3(-sin(transrotation.y), 0.0f,cos(transrotation.y)));
    mat3 rotationYaw = mat3(vec3(cos(transrotation.z),-sin(transrotation.z), 0.0f),
                            vec3(sin(transrotation.z), cos(transrotation.z),0.0f),
                            vec3(0.0f, 0.0f, 1.0f));
    position.xyz *= rotationRoll;
    position.xyz *= rotationPitch;
    position.xyz *= rotationYaw;

    position = transform * position;
    position.x += transposition.x;
    position.y += transposition.y;
    position.z += transposition.z;

    position *= view;

    fragPos = position;
    gl_Position = fragPos;
    vertexColor = aColor;
    normal = aNormal;
}
