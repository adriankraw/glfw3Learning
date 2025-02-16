#version 330 core
in vec3 Color;

out vec4 FragColor;
out vec4 aColor;
void main()
{
    aColor = vec4(Color.x,Color.y,Color.z,1.0f);
    //FragColor = aColor;
    FragColor = vec4(1.0f * gl_FragCoord.z, 0.5f * gl_FragCoord.z, 0.2f * gl_FragCoord.z, 1.0f);
}
