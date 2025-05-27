#version 330 core

uniform sampler2D texture1;

in vec3 vertexColor;
in vec3 normal;
in vec4 fragPos;
in vec2 TexCoord;

out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform Material material;

uniform vec3 lightColor = vec3(0.0f, 0.3f ,0.4f);
uniform vec3 lightPos = vec3(1.0f, -2.0f ,1.0f);

uniform vec3 viewPos;

void main()
{
    vec3 lightDir = normalize(lightPos - fragPos.xyz);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 color = vertexColor * (diff * lightColor);
    FragColor = vec4(vertexColor, 1.0f);
    //FragColor = texture(texture1, TexCoord);
    //FragColor = vec4(TexCoord,0.0f,1.0f);
}
