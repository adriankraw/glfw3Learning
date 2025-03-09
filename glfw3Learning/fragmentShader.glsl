#version 330 core
in vec3 Color;

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
    vec3 ambient = lightColor * material.ambient;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    //vec3 diffuse = lightColor * (diff * material.diffuse);
    vec3 diffuse = lightColor * (diff * Color);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = lightColor * (spec * material.specular);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(material.diffuse, 1.0f);
}
