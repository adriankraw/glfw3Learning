#version 330 core

in vec3 vertexColor;

out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

uniform vec3 Normal = vec3(0, 1, 0);
uniform vec3 FragPos;

uniform vec3 lightColor = vec3(1.0f,1.0f,1.0f);
uniform vec3 lightPos = vec3(1.0f, 1.0f ,1.0f);
uniform vec3 lightDir = vec3(0, 1.0f ,-1.0f );

uniform vec3 viewPos;

void main()
{
    FragColor = vec4(vertexColor, 1.0f);
}
