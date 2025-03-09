//opengl
#include "fbxsdk/scene/geometry/fbxlayer.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/trigonometric.hpp"
#include <array>
#include <cstddef>
#include <cstdio>
#include <iterator>
#include <ostream>
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB

#include <OpenGL/OpenGL.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//c++
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

//FBX SDK > assimp
#include <fbxsdk.h>
#include <fbxsdk/core/arch/fbxtypes.h>
#include <fbxsdk/fileio/fbxiosettings.h>
#include <fbxsdk/core/fbxmanager.h>
#include <fbxsdk/fileio/fbximporter.h>
#include <fbxsdk/fileio/fbxiosettingspath.h>
#include <fbxsdk/scene/fbxscene.h>
#include <fbxsdk/scene/geometry/fbxmesh.h>
#include <fbxsdk/scene/geometry/fbxnodeattribute.h>
#include <fbxsdk/utils/fbxgeometryconverter.h>
#include <fbxsdk/scene/shading/fbxsurfacematerial.h>

#define sizeFaktor 2
#define FPS 30
#define shaderpath "./"

//callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void error_callback(int code, const char* description);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//shader
const GLchar* readFromFile(const GLchar* pathToFile);
void GenerateShaderProgram(unsigned int *_shaderProgram);
std::string shadercontent = "";
unsigned int vertexShader,fragmentShader;

//modification
void glTransformArrays(glm::mat4 *trans, unsigned int *shaderProgram);

//fbx
class renderObject
{
public:
    unsigned int id;
    fbxsdk::FbxNodeAttribute::EType nodetype = fbxsdk::FbxNodeAttribute::EType::eNull;
    glm::mat4 translation;
    glm::mat4 rotation;
    std::vector<float> verticies;
    std::vector<glm::vec3> diffuse;
};

class FbxControler
{
    public:
        bool rotating = true;
        float rotationX = 0;
        float rotationY = 0;
        float rotationZ = 0;
        float zoomlevel = 1;
};

FbxScene* LoadFbxFile(const char& pFile);
void ImportMeshData(const FbxScene& scen,  renderObject& rObj, int& index);

//argv
std::string ArgvTrans = "-t";

FbxControler *controller = new FbxControler();

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

    //do an if argv contains -t 
    if(argc > 1 && ArgvTrans.compare(argv[1])==0)
    {
        glfwWindowHint (GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint (GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint (GLFW_FLOATING, GLFW_TRUE);
        glfwWindowHint (0x0002000D, 1); // -> GLFW_MOUSE_PASSTHROUGH
    }

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
    unsigned int VBOMaterial;
    unsigned int VAO;
    unsigned int shaderProgram = 0;
    
    //end

    glfwSetErrorCallback(error_callback);
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow( mode->width, mode->height, "OpenGl", NULL , NULL );
    
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback );
    
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
    glEnable(GL_DEPTH_TEST);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &VBOMaterial);
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    const char* fbxFileLocation = "./models/KitFoxChar1.fbx";
    const FbxScene *scen = LoadFbxFile(*fbxFileLocation);
    renderObject rObj[scen->GetRootNode()->GetChildCount(false)];
    for(int i = 0; i < scen->GetRootNode()->GetChildCount(false); ++i)
    {
        //looking for all the children is kind of mest up. ....->GetChildCount(false) is working fine
        ImportMeshData( *scen, rObj[i], i);
    }
    unsigned int objBufferSize = 0;
    for(renderObject const& i:rObj)
    {
        objBufferSize += i.verticies.size() * sizeof(float);
    }
    glBufferData(GL_ARRAY_BUFFER, objBufferSize, 0, GL_STATIC_DRAW);
    unsigned int lastbuffer = 0;
    for(renderObject &ro:rObj)
    {
        for(int i = 0; i < ro.verticies.size(); i+=3)
        {
            glBufferSubData(GL_ARRAY_BUFFER, lastbuffer, 3 * sizeof(float), &ro.verticies[i]);
            lastbuffer += 3 * sizeof(float);
        }
    }
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_UNIFORM_BUFFER, VBOMaterial);    
    objBufferSize = 0;
    for(renderObject const& i:rObj)
    {
        objBufferSize += i.diffuse.size() * sizeof(float);
    }
    glBufferData(GL_UNIFORM_BUFFER, objBufferSize, 0, GL_STATIC_DRAW);
    lastbuffer = 0;
    for(renderObject &ro:rObj)
    {
        for(int i = 0; i < ro.diffuse.size(); ++i)
        {
            glBufferSubData(GL_UNIFORM_BUFFER, lastbuffer, sizeof(glm::vec3), &ro.diffuse[i]);
            lastbuffer += 3 * sizeof(float);
        }
    }
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
    glEnableVertexAttribArray(1);



    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    GenerateShaderProgram(&shaderProgram);

    glm::mat4 trans = glm::mat4(1.0f);
    const glm::vec3 startPosi = glm::vec3(0.0f,-0.8f,0.0f);
    trans = glm::translate(trans, startPosi); 
    trans = glm::rotate(trans, glm::radians(220.0f), glm::vec3(0,1,0));

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glTransformArrays(&trans, &shaderProgram);

        // rendering done
        glClearColor(0.0f,0.0f,0.0f,0.0f); // Clear the buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        unsigned int lastvercount = 0;
        for( renderObject const& i:rObj)
        {
            if(i.diffuse.size() > 0)
            {
                glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), 0, 0.12f, 0.31f);
                glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), i.diffuse[0].x, i.diffuse[0].y, i.diffuse[0].z);
                glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"), 1.0f, 0.5f, 0.31f);
                glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 0.5f);
                glUniform3f(glGetUniformLocation(shaderProgram, "FragPos"), 1, 1, 1);
                glUseProgram(shaderProgram);
            }

            glDrawArrays(GL_TRIANGLES, lastvercount, (i.verticies.size()/3));
            lastvercount += (i.verticies.size()/3);
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
    if(controller->rotating && (controller->rotationX != 0 || controller->rotationY != 0 || controller->rotationZ != 0))
    {
        *trans = glm::rotate(*trans, glm::radians(1.0f), glm::vec3(controller->rotationX,controller->rotationY,controller->rotationZ));
        controller->rotationX = 0;
        controller->rotationY = 0;
        controller->rotationZ = 0;
    }
    *trans = glm::scale(*trans, glm::vec3(controller->zoomlevel));
    controller->zoomlevel = 1;

    unsigned int transformLoc = glGetUniformLocation(*shaderProgram, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(*trans));
}

FbxScene* LoadFbxFile(const char& pFile)
{
    FbxManager* SDKManager = FbxManager::Create();
    FbxIOSettings* ios = FbxIOSettings::Create(SDKManager, IOSROOT);
    SDKManager->SetIOSettings(ios);
    
    (SDKManager->GetIOSettings())->SetBoolProp(IMP_FBX_MATERIAL, true);
    (SDKManager->GetIOSettings())->SetBoolProp(IMP_FBX_MODEL, true);
    (SDKManager->GetIOSettings())->SetBoolProp(IMP_FBX_VISIBILITY, true);
    (SDKManager->GetIOSettings())->SetBoolProp(IMP_FBX_TEXTURE, true);
    (SDKManager->GetIOSettings())->SetBoolProp(IMP_FBX_LINK, false);
    (SDKManager->GetIOSettings())->SetBoolProp(IMP_FBX_SHAPE, false);
    (SDKManager->GetIOSettings())->SetBoolProp(IMP_FBX_GOBO, false);
    (SDKManager->GetIOSettings())->SetBoolProp(IMP_FBX_ANIMATION, false);
    (SDKManager->GetIOSettings())->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    
    FbxImporter* lImporter = FbxImporter::Create(SDKManager, "");
    

    bool lImportStatus = lImporter->Initialize(&pFile, -1, SDKManager->GetIOSettings());

    if(!lImportStatus)
    {
        printf("Call to FbxImporter::Initialize() failed. \n");

        if(lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFile) printf("Invalid File");
        if(lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion) printf("Invalid FileVersion");
        if(lImporter->GetStatus().GetCode() == FbxStatus::eInsufficientMemory) printf("Insufficient Memory");

        printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
        exit(-1);
    }
    
    FbxScene *scen = FbxScene::Create(SDKManager, "myScene");

    lImporter->Import(scen);
    lImporter->Destroy();
    FbxGeometryConverter converter(SDKManager);
    converter.Triangulate(scen, true, true);

    return scen;
}

void ImportMeshData(const FbxScene& scen,  renderObject& rObj, int& index)
{
    rObj.id = index;

    FbxNode* rootNode = scen.GetRootNode();
    FbxNode* node = rootNode->GetChild(index);
    if(node->GetChildCount() > 0)
    {
        for( size_t c=0; c < node->GetChildCount(); ++c)
        {
            //index+=c;
            //ImportMeshData(scen, rObj, index);
        }
    }
    if(node->GetNodeAttribute() == nullptr) return;

    rObj.nodetype = node->GetNodeAttribute()->GetAttributeType();
    if(rObj.nodetype == FbxNodeAttribute::eMesh)
    {
        FbxMesh* mesh = node->GetMesh();
        
        std::cout << "nodeName " << node->GetName() << std::endl;
        for (int m = 0; m < mesh->GetElementMaterialCount();++m)
        {
            FbxGeometryElementMaterial* lMaterialElement = mesh->GetElementMaterial(m);
            std::cout << "mesh:" <<mesh->GetName() << std::endl;
            switch(lMaterialElement->GetMappingMode())
            {
                case fbxsdk::FbxLayerElement::EMappingMode::eAllSame:
                {
                    FbxSurfaceMaterial* lMaterial = node->GetMaterial(lMaterialElement->GetIndexArray().GetAt(0));
                    FbxDouble3 diffColor = FbxPropertyT<FbxDouble3>(lMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse));
                    for(int i; i < node->GetMesh()->GetPolygonCount(); ++i)
                    {
                        //rObj.diffuse.push_back(glm::vec3(diffColor[0],diffColor[1],diffColor[2]));
                        rObj.diffuse.push_back(glm::vec3(255,0,0));
                    }
                }
                    break;
                case fbxsdk::FbxLayerElement::EMappingMode::eByEdge:
                    break;
                case fbxsdk::FbxLayerElement::EMappingMode::eByControlPoint:
                    break;
                case fbxsdk::FbxLayerElement::EMappingMode::eByPolygon:
                {
                    for (int mat = 0; mat < lMaterialElement->GetIndexArray().GetCount(); ++mat)
                    {
                        FbxSurfaceMaterial* lMaterial = node->GetMaterial(lMaterialElement->GetIndexArray().GetAt(mat));
                        FbxDouble3 diffColor = FbxPropertyT<FbxDouble3>(lMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse));
                        rObj.diffuse.push_back(glm::vec3(diffColor[0],diffColor[1],diffColor[2]));
                    }
                }
                    break;
                case fbxsdk::FbxLayerElement::EMappingMode::eByPolygonVertex:
                    break;
                case fbxsdk::FbxLayerElement::EMappingMode::eNone:
                    break;
            }
        }
        
        

        std::cout << "nodeName " << node->GetName() << std::endl;
        int verticiesCount = mesh->GetPolygonCount();
        for(int p = 0; p < verticiesCount; ++p)
        {
            for(int poligonvertex = 0; poligonvertex < mesh->GetPolygonSize(p); ++poligonvertex)
            {
                int vertIndies = mesh->GetPolygonVertex(p, poligonvertex);
                FbxVector4 vec = mesh->GetControlPointAt(vertIndies);
                rObj.verticies.push_back(vec.mData[0]);
                rObj.verticies.push_back(vec.mData[1]);
                rObj.verticies.push_back(vec.mData[2]);
                //open3Mod    
                //verscueh das über pointer zu machen mit m alloc re alloc etc.
            }
        }
        std::cout << "nodeName " << node->GetName() << std::endl;
    }
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
    if(key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        controller->rotating = !controller->rotating;
    }
    if(key == GLFW_KEY_M && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        controller->zoomlevel -=0.5;
    }
    if(key == GLFW_KEY_P && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        controller->zoomlevel +=0.5;
    }    
    if(key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        controller->rotationX = 1;
    }
    if(key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        controller->rotationX =-1;
    }    
    if(key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        controller->rotationY = 1;
    }
    if(key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        controller->rotationY = -1;
    }    
    if(key == GLFW_KEY_Q && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        controller->rotationZ = 1;
    }
    if(key == GLFW_KEY_E && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        controller->rotationZ = -1;
    }
    if(key == GLFW_KEY_F1 && (action == GLFW_PRESS || action == GLFW_REPEAT))
    {
        //would like to export thecurrent trans(form) matrix
    }
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    controller->zoomlevel = 1+(xoffset/50);
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
    
    const char *vertexShaderSource = readFromFile("./vertexShader.glsl");
    
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    const char *fragmentShaderSource = readFromFile("./fragmentShader.glsl");
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
