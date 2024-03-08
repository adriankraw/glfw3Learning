//opengl
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#include "../include/GLFW/glfw3.h"
#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"

//c++
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <list>

//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define sizeFaktor 2
#define FPS 15
#define shaderpath "../"

//callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void error_callback(int code, const char* description);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
//shader
const GLchar* readFromFile(const GLchar* pathToFile);
void GenerateShaderProgram(unsigned int *_shaderProgram);
std::string shadercontent = "";
unsigned int vertexShader,fragmentShader;

//modification
void glTransformArrays(glm::mat4 *trans, unsigned int *shaderProgram);

//assimp
class renderObject
{
public:
    unsigned int id;
    glm::mat4 translation;
    glm::mat4 rotation;
    float vertcount;
    std::vector<float> verticies;
};
Assimp::Importer importer;
const aiScene LoadFileWithAssimp(const std::string& pFile, const aiScene *scen);
void InportMeshData(const aiScene *scen,  renderObject& rObj, int& index);

int main(int argc, char** argv)
{
    GLFWwindow* window;
    /* Initialize the library */
    if ( !glfwInit() )
    {
        return -1;
    } 
    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint (GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint (GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint (GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint (0x0002000D, 1); // -> GLFW_MOUSE_PASSTHROUGH
    
    int count;
    GLFWmonitor **monitors = glfwGetMonitors(&count);


    for(int i = 0; i < count; ++i)
    {
        GLFWmonitor &moniforName = **(monitors+i);
        std::cout << i<< ": " << glfwGetMonitorName(&moniforName) << std::endl;
        std::cout << glfwGetVideoMode(&moniforName)->width << ":" << glfwGetVideoMode(&moniforName)->height <<std::endl;
    }
    int monitorIndex;
    std::cout << "Enter monitor Index: ";
    
    //scanf(" %1i", &monitorIndex);
    monitorIndex = 0;
    
    GLFWmonitor &monitor = **(monitors+monitorIndex);
    const GLFWvidmode *mode = glfwGetVideoMode(&monitor);
    std::cout << "   ->" << mode->width << ":" << mode->height <<std::endl;
    glfwWindowHint (GLFW_RED_BITS, mode->redBits);
    glfwWindowHint (GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint (GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint (GLFW_REFRESH_RATE, mode->refreshRate);
    
    
    unsigned int VBO;
    unsigned int VAO;
    unsigned int shaderProgram = 0;
    
    //end

    glfwSetErrorCallback(error_callback);
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow( mode->width, mode->height, "OpenGl", NULL , NULL );
    
    glfwSetKeyCallback(window, key_callback);
    
    if (!window)
    {
        glfwTerminate();
        std::cout << "Failes to create GLFW window" << std::endl;
        return -1;
    }
    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    std::cout << glfwGetVersionString()<< std::endl;
    
    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glEnable(GL_DEPTH_TEST);
    
    const aiScene scen = LoadFileWithAssimp("./models/cube.fbx", &scen);
    renderObject rObj[scen.mNumMeshes];
    for(int i = 0; i < scen.mNumMeshes; ++i)
    {
        InportMeshData( &scen, rObj[i], i);
    }
    
    //tesselation glu
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    unsigned int addedVerticies = 0;
    for(renderObject const& i:rObj)
    {
        addedVerticies += i.vertcount;
    }
    glBufferData(GL_ARRAY_BUFFER, (addedVerticies * sizeof(float)), 0, GL_STATIC_DRAW);
    unsigned int lastbuffer = 0;
    for(renderObject &ro:rObj)
    {
        glBufferSubData(GL_ARRAY_BUFFER, lastbuffer, ro.verticies.size() * sizeof(float), &ro.verticies[0]);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        lastbuffer += (ro.verticies.size() * sizeof(float));
    }
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    GenerateShaderProgram(&shaderProgram);
    
    glm::mat4 trans = glm::mat4(1.0f);
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glUseProgram(shaderProgram);
        /* Render here */
        glTransformArrays(&trans, &shaderProgram);

        // rendering done
        glClearColor(0.0f,0.0f,0.0f,0.0f); // Clear the buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        unsigned int lastvercount = 0;
        for( renderObject const& i:rObj)
        {
            glDrawArrays(GL_TRIANGLES, lastvercount, (i.vertcount/3));
            lastvercount += (i.vertcount/3);
        }
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        /* Poll for and process events */
        glfwPollEvents();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000/FPS));
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
    return 0;
}

void glTransformArrays(glm::mat4 *trans, unsigned int *shaderProgram)
{
    *trans = glm::rotate(*trans, glm::radians(1.0f), glm::vec3(0.0f,1.0f,0.0f));
    *trans = glm::rotate(*trans, glm::radians(1.0f), glm::vec3(1.0f,0.0f,0.0f));
    
    unsigned int transformLoc = glGetUniformLocation(*shaderProgram, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(*trans));
}

const aiScene LoadFileWithAssimp(const std::string& pFile, const aiScene *scen)
{
    scen = importer.ReadFile(pFile,
    aiProcess_CalcTangentSpace |
    aiProcess_Triangulate |
    aiProcess_JoinIdenticalVertices |
    aiProcess_SortByPType
    );
    
    if (!scen)
    {
        std::cout << "assimp fcked up" << std::endl;
    }
    return *scen;
}

void InportMeshData(const aiScene *scen,  renderObject& rObj, int& index)
{
    rObj.id = index;
    aiMesh* mesh = scen->mMeshes[index];
    for(int f = 0; f < mesh->mNumFaces; ++f)
    {
        aiFace face = mesh->mFaces[f];
        for(int vi = 0; vi < face.mNumIndices; ++vi)
        {
            aiVector3D verts = mesh->mVertices[face.mIndices[vi]];
            rObj.verticies.push_back(verts.x/sizeFaktor);
            rObj.verticies.push_back(verts.y/sizeFaktor);
            rObj.verticies.push_back(verts.z/sizeFaktor);
            
            //verscueh das über pointer zu machen mit m alloc re alloc etc.
        }
    }
    printf("%i: meshes %i\n",index, (int)rObj.verticies.size());
    rObj.vertcount = rObj.verticies.size();
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
void error_callback(int code, const char* description)
{
    std::cout << code << "  " << description << std::endl;
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}
const GLchar* readFromFile(const GLchar* pathToFile)
{
    shadercontent = "";
    std::ifstream fileStream;
    fileStream.open(pathToFile, std::ios::in);
    
    if(!fileStream.is_open())
    {
        std::cout << "something went wrong" << std::endl;
    }else
    {
        std::string line;
        while (getline(fileStream,line))
        {
            shadercontent.append(line + "\n");
        }
    }
    fileStream.close();
    return shadercontent.c_str();
}

void GenerateShaderProgram(unsigned int *_shaderProgram)
{
    
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    
    const char *vertexShaderSource = readFromFile("/Users/olantern/Documents/CodeProjects/OpenGLDez2023/emptyWindow/glfw3Learning/glfw3Learning/vertexShader.glsl");
    
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    const char *fragmentShaderSource = readFromFile("/Users/olantern/Documents/CodeProjects/OpenGLDez2023/emptyWindow/glfw3Learning/glfw3Learning/fragmentShader.glsl");
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    int sucess;
    char infoLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &sucess);
    if(!sucess)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER:FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &sucess);
    if(!sucess)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    *_shaderProgram = glCreateProgram();
    
    glAttachShader(*_shaderProgram, vertexShader);
    glAttachShader(*_shaderProgram, fragmentShader);
    glLinkProgram(*_shaderProgram);
    
    glGetShaderiv(*_shaderProgram, GL_LINK_STATUS, &sucess);
    if(!sucess)
    {
        glGetShaderInfoLog(*_shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    glUseProgram(*_shaderProgram);
}
