//opengl
#include "fbxsdk/core/math/fbxvector2.h"
#include "fbxsdk/scene/geometry/fbxlayer.h"
#include "fbxsdk/scene/geometry/fbxnode.h"
#include "fbxsdk/scene/shading/fbxfiletexture.h"
#include "fbxsdk/scene/shading/fbxtexture.h"
#include "glm/detail/qualifier.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include <cstddef>
#include <cstdio>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>
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

//imgui 
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define sizeFaktor 2
#define FPS 30
#define shaderpath "./"

//callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void error_callback(int code, const char* description);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void windowFocus_callback(GLFWwindow* window, int focused);
//shader
const GLchar* readFromFile(const GLchar* pathToFile);
void GenerateShaderProgram(unsigned int &_shaderProgram);
std::string shadercontent = "";
unsigned int vertexShader,fragmentShader;

//Camera
void SetupCamera(glm::mat4 *view);
//modification
void glTransformArrays(glm::mat4 *trans, unsigned int *shaderProgram);

//fbx
class renderObject
{
public:
    unsigned int id;
    const char* name;
    fbxsdk::FbxNodeAttribute::EType nodetype = fbxsdk::FbxNodeAttribute::EType::eNull;
    glm::mat4 translation;
    glm::mat4 rotation;
    std::vector<unsigned int> vertIndices;
    std::vector<float> verticies;
    std::vector<float> diffuse;
    std::vector<float> normal;
};

class GameObject
{
public:
    unsigned int renderOrder;
    unsigned int rObjCount;
    glm::vec3 transform;
    glm::vec3 rotation;
    glm::vec3 scale;
    std::vector<renderObject> rObjects;
    FbxScene* fbxScene;
    GameObject(int _renderOrder, glm::vec3 _trans, glm::vec3 _rot, glm::vec3 _scale) 
    {
        renderOrder = _renderOrder;
        transform = _trans;
        rotation = _rot;
        scale = _scale;
        rObjCount = 0;
    }
};

//currently used for handling keyboardinput
class FbxControler
{
    public:
        bool rotating = true;
        float rotationX = 0;
        float rotationY = 0;
        float rotationZ = 0;
        float zoomlevel = 1;
};

const std::string AttributTypeToString[]  = {
        "eUnknown",
        "eNull",
        "eMarker",
        "eSkeleton", 
        "eMesh", 
        "eNurbs", 
        "ePatch",
        "eCamera", 
        "eCameraStereo",
        "eCameraSwitcher",
        "eLight",
        "eOpticalReference",
        "eOpticalMarker",
        "eNurbsCurve",
        "eTrimNurbsSurface",
        "eBoundary",
        "eNurbsSurface",
        "eShape",
        "eLODGroup",
        "eSubDiv",
        "eCachedEffect",
        "eLine"
};

GLenum glCheckError_(const char *file, int line, const char * function)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
            default: error = "UNKNOWN"; break;
        }
        std::cout << error << " | " << file << ":" << function << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
//#define glCheckError() glCheckError_(__FILE__, __LINE__) 
#define glCheckError() glCheckError_(__FILE__, __LINE__, __FUNCTION__) 

FbxScene* LoadFbxFile(const char& pFile);
void ImportMeshData(FbxNode* scen,  std::vector<renderObject>* rObj, int& index);

void RenderImGuiNodeHierarchy(FbxNode * node);

//argv
const std::string ArgvTrans = "-t";
const char* fbxGroundLocation = "./models/DefaultGround.fbx";
const char* fbxFileLocation = "./models/KitFoxChar1.fbx";
//const char* fbxFileLocation = "./models/untitled1.fbx";
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
    if((argc > 1 && ArgvTrans.compare(argv[1])==0) || true)
    {
        glfwWindowHint (GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint (GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint (GLFW_FLOATING, GLFW_TRUE);
        glfwWindowHint (0x0002000D, 0); // -> GLFW_MOUSE_PASSTHROUGH
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

    unsigned int VBO, EBO;
    unsigned int VBOMaterial;
    unsigned int VAO;
    unsigned int shaderProgram = 0;
    unsigned int fbo;
    unsigned int rbo;
    
    glfwSetErrorCallback(error_callback);

    window = glfwCreateWindow( mode->width, mode->height, "OpenGl", NULL , NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);   

    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetWindowFocusCallback(window, windowFocus_callback);
    //glfwSetWindowAspectRatio(window, 21, 9);

     
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");
    // Our state
    bool show_demo_window = true;
    bool imGuiWindowHierarchy = true;
    bool imGuiWindowTransform = true;
    bool imGuiWindowRender = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


    if (!window)
    {
        glfwTerminate();
        std::cout << "Failes to create GLFW window" << std::endl;
        return -1;
    }
    std::cout << glfwGetVersionString()<< std::endl;
    
    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    glEnable(GL_DEPTH_TEST);
    glCheckError();

    glGenFramebuffers(1, &fbo);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    unsigned int mainRenderTexture;
    glGenTextures(1, &mainRenderTexture);
    glBindTexture(GL_TEXTURE_2D, mainRenderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mode->width/2, mode->height/2, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mainRenderTexture, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mode->width/2, mode->height/2);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glCheckError();

    std::vector<GameObject> gObj;
    
    gObj.emplace_back(GameObject(1, glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1,1,1)));

    FbxScene *Groundscen = LoadFbxFile(*fbxGroundLocation);
    gObj.back().rObjCount = Groundscen->GetNodeCount();
    gObj.back().fbxScene = Groundscen;
    std::cout << "NodeCount: " << gObj.back().rObjCount << std::endl;
    for (int i = 0; i < gObj.back().rObjCount; ++i)
    {
        ImportMeshData( Groundscen->GetNode(i), &(gObj.back().rObjects), i);
    }

    gObj.emplace_back(GameObject(1, glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(1,1,1)));
    FbxScene *scen = LoadFbxFile(*fbxFileLocation);
    gObj.back().rObjCount = scen->GetNodeCount();
    gObj.back().fbxScene = scen;
    std::cout << "NodeCount: " << gObj.back().rObjCount << std::endl;
    for (int i = 0; i < gObj.back().rObjCount; ++i)
    {
        ImportMeshData( scen->GetNode(i), &(gObj.back().rObjects), i);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBOMaterial);
    glCheckError();

    glm::mat4 view = glm::mat4(1.0f);
    SetupCamera(&view);
    glCheckError();

    GenerateShaderProgram(shaderProgram);
    glCheckError();
    glm::mat4 trans = glm::mat4(1.0f);
    glm::vec3 transposition = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 transRotation = glm::vec3(-1.0f, 0.0f, 0.0f);
    glm::vec3 transScaling = glm::vec3(0.5f, 0.5f, 0.5f);

    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
        glCheckError();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // clear last frame
        glClearColor(0.0f,0.0f,0.0f,0.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCheckError();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        float f = 0.0f;
        int counter = 0;

        
        if(imGuiWindowHierarchy)
        {
            //ImGui::SetNextWindowPos(ImVec2(0,0));
            //ImGui::SetNextWindowSizeConstraints(ImVec2(50.0f,400.f), ImVec2(-1,-1));
            ImGui::Begin("Hierarchy");                          
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            for (unsigned int i = 0; i < gObj.size(); ++i)
            {
                GameObject* go = &gObj[i];
                std::string name = go->fbxScene->GetName();
                name.append(" ");
                name.append(std::to_string(i));
                if(ImGui::TreeNode(name.c_str()))
                {
                    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                    RenderImGuiNodeHierarchy(go->fbxScene->GetRootNode());
                    ImGui::TreePop();
                }
            }
            ImGui::End();
        }
        if(imGuiWindowTransform)
        {
            //ImGui::SetNextWindowPos(ImVec2(0,0));
            //ImGui::SetNextWindowSizeConstraints(ImVec2(50.0f,200.f), ImVec2(-1,-1));
            ImGui::Begin("Transform");
            ImGui::Spacing();
            ImGui::InputFloat3("Position",(float*)&transposition);
            unsigned int TranspositionLoc = glGetUniformLocation(shaderProgram, "transposition");
            glUniform3f(TranspositionLoc, transposition.x, transposition.y, transposition.z);
            ImGui::Spacing();
            ImGui::InputFloat3("Rotation",(float*)&transRotation);
            unsigned int TransRotationLoc = glGetUniformLocation(shaderProgram, "transrotation");
            glUniform3f(TransRotationLoc, transRotation.x, transRotation.y, transRotation.z);
            ImGui::Spacing();
            ImGui::InputFloat3("Scale",(float*)&transScaling);
            unsigned int TransScalingLoc = glGetUniformLocation(shaderProgram, "transscaling");
            glUniform3f(TransScalingLoc, transScaling.x, transScaling.y, transScaling.z);
            ImGui::End();
        }
        if(imGuiWindowRender)
        {
            ImGui::Begin("Render");
            ImGui::Image((ImTextureID)mainRenderTexture, ImVec2(mode->width/2.0f,mode->height/2.0f), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::End();
        }

        ImGui::Begin("Settings");
        ImGui::Checkbox("Hierarchy", &imGuiWindowHierarchy);     
        ImGui::Checkbox("Transform", &imGuiWindowTransform); 
        ImGui::Checkbox("Render in Window", &imGuiWindowRender); 
        ImGui::Checkbox("Demo Window", &show_demo_window);     
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f); 
        ImGui::ColorEdit3("clear color", (float*)&clear_color); 
        if (ImGui::Button("Button"))                           
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glTransformArrays(&trans, &shaderProgram);

        glCheckError();
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Framebuffer not Complete" << std::endl;
        }
        if(imGuiWindowRender)
        {
            glViewport(0, 0, mode->width/2, mode->height/2);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glBindTexture(GL_TEXTURE_2D, mainRenderTexture);
            glClearColor(clear_color.x,clear_color.y,clear_color.z, clear_color.w); 
        }else{
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0,0,mode->width,mode->height);
            glBindTexture(GL_TEXTURE_2D, 0);
            glClearColor(0.0f,0.0f,0.0f,0.0f);
        }
        glUseProgram(shaderProgram);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(VAO);

        glCheckError();
        for (GameObject const &go: gObj)
        {
            for (int i = 0; i < go.rObjCount; ++i)
            {
                if (go.rObjects[i].nodetype !=FbxNodeAttribute::eMesh) continue;

                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, go.rObjects[i].verticies.size() * sizeof(float), &go.rObjects[i].verticies[0], GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
                glEnableVertexAttribArray(0);
                glCheckError();

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, go.rObjects[i].vertIndices.size() * sizeof(unsigned int), &go.rObjects[i].vertIndices[0], GL_STATIC_DRAW);
                glCheckError();

                glBindBuffer(GL_ARRAY_BUFFER, VBOMaterial);    
                glBufferData(GL_ARRAY_BUFFER, (go.rObjects[i].diffuse.size() + go.rObjects[i].normal.size()) * sizeof(float)  , 0, GL_STATIC_DRAW);
                glBufferSubData(GL_ARRAY_BUFFER, 0, go.rObjects[i].normal.size() * sizeof(float), &go.rObjects[i].diffuse[0]);
                glBufferSubData(GL_ARRAY_BUFFER, go.rObjects[i].diffuse.size() * sizeof(float), go.rObjects[i].normal.size() * sizeof(float), &go.rObjects[i].normal[0]);
                glCheckError();

                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)(go.rObjects[i].diffuse.size()*sizeof(float)));
                glEnableVertexAttribArray(2);
                glCheckError();
                
                glDrawElements(GL_TRIANGLES, go.rObjects[i].vertIndices.size(), GL_UNSIGNED_INT, 0);
                glCheckError();

                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
                glDisableVertexAttribArray(2);
            }
        }
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        ImGui::EndFrame();
        //std::this_thread::sleep_for(std::chrono::milliseconds(1000/FPS));
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &VBOMaterial);
    glDeleteProgram(shaderProgram);
    glDeleteFramebuffers(1, &fbo);
    
    glfwTerminate();
    return 0;
}

void glTransformArrays(glm::mat4 *trans, unsigned int *shaderProgram)
{
    if(controller->rotating && (controller->rotationX != 0 || controller->rotationY != 0 || controller->rotationZ != 0))
    {
        *trans = glm::rotate(*trans, glm::radians(2.0f), glm::vec3(controller->rotationX,controller->rotationY,controller->rotationZ));
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
    bool isFine = converter.Triangulate(scen, true, false);
    if (!isFine) std::cout << "Triangulate failed" << std::endl;
    //isFine = converter.SplitMeshesPerMaterial(scen, true);
    //if (!isFine) std::cout << "converter failed to split" << std::endl;
    
    return scen;
}
void ImportMeshData(FbxNode *node, std::vector<renderObject>* renderList, int& index)
{
    if(node->GetMesh() == nullptr) 
    {
        return;
    }
    std::cout << "----------------------" << std::endl;
    std::cout << "node:" << node->GetName() << "type: " << AttributTypeToString[node->GetNodeAttribute()->GetAttributeType()] << std::endl;
    renderObject *rObj = new renderObject();
    rObj->id = index;
    rObj->name = node->GetName();
    rObj->nodetype = node->GetNodeAttribute()->GetAttributeType();

    FbxMesh* mesh = node->GetMesh();
    std::cout << "mesh:" << mesh->GetName() << std::endl;
    std::cout << "layer:" << mesh->GetLayerCount() << std::endl;
    for (unsigned int i = 0; i < node->GetMaterialCount(); ++i)
    {
        FbxSurfaceMaterial* material = node->GetMaterial(i);
        FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
        if(prop.IsValid())
        {
            for(unsigned int t = 0; t < prop.GetSrcObjectCount<FbxFileTexture>();++t)
            {
                FbxFileTexture* texture = prop.GetSrcObject<FbxFileTexture>(i);
                if (texture)
                {
                    std::cout << texture->GetFileName() << std::endl;
                }
            }
        }

    }

    for(int v = 0;v < mesh->GetControlPointsCount(); ++v)
    {
        FbxVector4 vec = mesh->GetControlPointAt(v);
        rObj->verticies.push_back(vec.mData[0]);
        rObj->verticies.push_back(vec.mData[1]);
        rObj->verticies.push_back(vec.mData[2]);
    }

    int verticiesCount = mesh->GetPolygonCount();
    for(int p = 0; p < verticiesCount; ++p)
    {
        for(int poligonvertex = 0; poligonvertex < mesh->GetPolygonSize(p); ++poligonvertex)
        {
            int vertIndies = mesh->GetPolygonVertex(p, poligonvertex);
            rObj->vertIndices.push_back(vertIndies);
        }
    }
    std::cout << "mat Count" << mesh->GetElementMaterialCount() << std::endl;

    for (int uv_index = 0; uv_index < mesh->GetElementUVCount(); ++uv_index)
    {
        FbxGeometryElementUV* leUv = mesh->GetElementUV(uv_index);
        switch(leUv->GetMappingMode())
        {
            case fbxsdk::FbxLayerElement::eAllSame:
                std::cout << "texture AllSame" << std::endl;
            break;
            case fbxsdk::FbxLayerElement::eByEdge:
                std::cout << "texture Edge" << std::endl;
            break;
            case fbxsdk::FbxLayerElement::eByControlPoint:
                std::cout << "texture ControlPoint" << std::endl;
            break;
            case fbxsdk::FbxLayerElement::eByPolygon:
                std::cout << "texture Polygon" << std::endl;
            break;
            case fbxsdk::FbxLayerElement::eByPolygonVertex:
            {
                std::cout << "uv PolyVertex" << std::endl;
                FbxVector2 uv_id;
                switch(leUv->GetReferenceMode())
                {
                    case fbxsdk::FbxGeometryElement::eDirect:
                        {
                            std::cout << "eDirect" << std::endl;
                            uv_id = leUv->GetDirectArray().GetAt(uv_index);
                        }
                    break;
                    case fbxsdk::FbxGeometryElement::eIndexToDirect:
                        {
                            std::cout << "eIndexToDirect" << std::endl;
                            int id = leUv->GetIndexArray().GetAt(uv_index);
                            uv_id = leUv->GetDirectArray().GetAt(id);
                        }
                    break;
                    default:
                    break;
                }
            }
            break;
            case fbxsdk::FbxLayerElement::eNone:
                std::cout << "texture None" << std::endl;
            break;
        }
    }

    for (int m = 0; m < mesh->GetElementMaterialCount();++m)
    {
        FbxGeometryElementMaterial* lMaterialElement = mesh->GetElementMaterial(m);
        switch(lMaterialElement->GetMappingMode())
        {
            case fbxsdk::FbxLayerElement::EMappingMode::eAllSame:
            {
                rObj->diffuse.resize(mesh->GetControlPointsCount() * 3, 0.0f);
                rObj->normal.resize(mesh->GetControlPointsCount() * 3, 0.0f);

                for(unsigned int pol = 0; pol < mesh->GetPolygonCount(); ++pol)
                {
                    const int lMaterialIndex = lMaterialElement->GetIndexArray().GetAt(pol);
                    FbxSurfaceMaterial* lMaterial = node->GetMaterial(lMaterialIndex);
                    FbxDouble3 diffColor = FbxPropertyT<FbxDouble3>(lMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse));
                    rObj->diffuse.emplace_back(diffColor[0]);
                    rObj->diffuse.emplace_back(diffColor[1]);
                    rObj->diffuse.emplace_back(diffColor[2]);

                    for(unsigned int vert = 0; vert < mesh->GetPolygonSize(pol); ++vert)
                    {
                        int cpIndex = mesh->GetPolygonVertex(pol, vert);

                        FbxVector4 cpNormal;
                        mesh->GetPolygonVertexNormal(pol, vert, cpNormal);
                        rObj->normal[cpIndex*3+0] = cpNormal[0];
                        rObj->normal[cpIndex*3+1] = cpNormal[1];
                        rObj->normal[cpIndex*3+2] = cpNormal[2];
                    }
                }
                std::cout<< "----------All Same  " << rObj->diffuse.size()/3 << std::endl;
            }
                break;
            case fbxsdk::FbxLayerElement::EMappingMode::eByEdge:
                break;
            case fbxsdk::FbxLayerElement::EMappingMode::eByControlPoint:
                std::cout << "eByControlPoints found" << "\n";
                break;
            case fbxsdk::FbxLayerElement::EMappingMode::eByPolygon:
            {
                rObj->diffuse.resize(mesh->GetControlPointsCount() * 3, 0.0f);
                rObj->normal.resize(mesh->GetControlPointsCount() * 3, 0.0f);
                for(unsigned int pol = 0; pol < mesh->GetPolygonCount(); ++pol)
                {
                    int matIndex = 0;
                    
                    if (lMaterialElement->GetReferenceMode() == FbxGeometryElement::eDirect)
                    {
                        matIndex = pol;
                    }else if (lMaterialElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                    {
                        matIndex = lMaterialElement->GetIndexArray().GetAt(pol);
                    }
                    FbxSurfaceMaterial* lMaterial = node->GetMaterial(matIndex);
                    FbxDouble3 diffColor = FbxPropertyT<FbxDouble3>(lMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse));
                    //FbxDouble3 normalMap    = FbxPropertyT<FbxDouble3>(lMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap));

                    for(unsigned int vert = 0; vert < mesh->GetPolygonSize(pol); ++vert)
                    {
                        int cpIndex = mesh->GetPolygonVertex(pol, vert);
                        rObj->diffuse[cpIndex*3 + 0] = diffColor[0];
                        rObj->diffuse[cpIndex*3 + 1] = diffColor[1];
                        rObj->diffuse[cpIndex*3 + 2] = diffColor[2];

                        FbxVector4 cpNormal;
                        mesh->GetPolygonVertexNormal(pol, vert, cpNormal);
                        rObj->normal[cpIndex*3+0] = cpNormal[0];
                        rObj->normal[cpIndex*3+1] = cpNormal[1];
                        rObj->normal[cpIndex*3+2] = cpNormal[2];
                    }
                }
                std::cout<< "--------By Poligon  " << rObj->diffuse.size()/3 << std::endl;
            }
                break;
            case fbxsdk::FbxLayerElement::EMappingMode::eByPolygonVertex:
                break;
            case fbxsdk::FbxLayerElement::EMappingMode::eNone:
                break;
        }
    renderList->emplace_back(*rObj);
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
    if(key == GLFW_KEY_F2 && (action == GLFW_PRESS))
    {
        glfwSetWindowAttrib(window, 0x0002000D, 0);
    }
    if(key == GLFW_KEY_F3 && (action == GLFW_PRESS))
    {
        glfwSetWindowAttrib(window, 0x0002000D, 1);
    }
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    controller->zoomlevel = 1+(xoffset/50);
}
void windowFocus_callback(GLFWwindow* window, int focused)
{
    if(focused)
    {
        std::cout << "window is now focused" << std::endl;
    }else{
        std::cout << "window is not focused" << std::endl;
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

void GenerateShaderProgram(unsigned int &_shaderProgram)
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
    _shaderProgram = glCreateProgram();

    glAttachShader(_shaderProgram, vertexShader);
    glAttachShader(_shaderProgram, fragmentShader);
    glLinkProgram(_shaderProgram);
    
    glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &sucess);
    if(!sucess)
    {
        glGetProgramInfoLog(_shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    glUseProgram(_shaderProgram);
}

void SetupCamera(glm::mat4 *view)
{
    glm::vec3 cameraPos = glm::vec3(0.0f,0.0f,-3.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 cameraDirection = glm::normalize(cameraTarget-cameraPos);
    
    glm::vec3 up = glm::vec3(0.0f,1.0f,0.0f);
    glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
    glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

    //glm requires a position, target and up vector;
    *view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
}

void RenderImGuiNodeHierarchy(FbxNode * node)
{
    std::string c = node->GetName(); 
    if(node->GetNodeAttribute())
    {
        c.append(" => ");
        c.append(AttributTypeToString[node->GetNodeAttribute()->GetAttributeType()]);
    }
    
    if(node->GetChildCount() > 0)
    {
        if(ImGui::TreeNode(c.c_str()))
        {
            for(unsigned int c = 0; c < node->GetChildCount(); ++c)
            {
                RenderImGuiNodeHierarchy(node->GetChild(c));
            }
            ImGui::TreePop();
        }
    }else{
        ImGui::Text("%s", c.c_str());
    }
}
