#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "texture.h"
#include "meshmanager.h"
#include "texturemanager.h"
#include "object.h"

#include <iostream>
#include <cmath>
#include <tuple>

namespace
{
    constexpr size_t windowWidth = 1440;
    constexpr size_t windowHeight = 810;

    const char *vertexShaderSource = "./shaders/vertex.vs";
    const char *fragmentShaderSource = "./shaders/fragment.fs";

    const char *lightVertexShaderSource = "./shaders/light_vertex.vs";
    const char *lightFragmentShaderSource = "./shaders/light_fragment.fs";

    const char *axesVertexShaderSource = "./shaders/axis_vertex.vs";
    const char *axesFragmentShaderSource = "./shaders/axis_fragment.fs";
    const char *axesGeometryShaderSource = "./shaders/axis_geometry.gs";

    Camera camera;
    float lastX{0.0f};
    float lastY{0.0f};
    float deltaTime{0.0f};
    float previousTime{0.0f};

    const float lightRotationRadius = 25.0f;
}

void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// TODO:move stuff to the camera or set up some observer pipeline
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    camera.processKeyboard(MovementInput{
                               glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS,
                               glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS,
                               glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS,
                               glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS,
                               glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS,
                               glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS},
                           deltaTime);
}

void mouseCallback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        camera.processMouseMovement(xoffset, yoffset);
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.processMouseScroll(static_cast<float>(yoffset));
}

MeshManager *MeshManager::_instance = nullptr;
TextureManager *TextureManager::_instance = nullptr;

int main(int argc, const char *argv[])
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *mainWindow = glfwCreateWindow(windowWidth, windowHeight, "Engine", NULL, NULL);
    if (mainWindow == NULL)
    {
        std::cerr << "Window creation failed" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(mainWindow);
    glfwSetFramebufferSizeCallback(mainWindow, framebufferResizeCallback);
    glfwSetCursorPosCallback(mainWindow, mouseCallback);
    glfwSetScrollCallback(mainWindow, scrollCallback);
    glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, windowWidth, windowHeight);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    // glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    //// Cubes buffers

    MeshManager::instance()->registerMesh("simple_cube",
                                          Mesh{
                                              {{-5.0f, -5.0f, -5.0f, 0.1f, 0.0f, -5.0f, -5.0f, -5.0f},
                                               {-5.0f, -5.0f, 5.0f, -0.04f, 1.0f, -5.0f, -5.0f, 5.0f},
                                               {-5.0f, 5.0f, 5.0f, 0.3f, 1.0f, -5.0f, 5.0f, 5.0f},
                                               {-5.0f, 5.0f, -5.0f, -0.5f, 0.0f, -5.0f, 5.0f, -5.0f},

                                               {5.0f, -5.0f, -5.0f, 0.31f, 1.0f, 5.0f, -5.0f, -5.0f},
                                               {5.0f, -5.0f, 5.0f, -0.15f, 1.0f, 5.0f, -5.0f, 5.0f},
                                               {5.0f, 5.0f, 5.0f, 0.014f, 0.0f, 5.0f, 5.0f, 5.0f},
                                               {5.0f, 5.0f, -5.0f, -0.65f, 0.0f, 5.0f, 5.0f, -5.0f}},

                                              {0, 1, 2,
                                               2, 3, 0,

                                               0, 4, 5,
                                               5, 1, 0,

                                               4, 5, 6,
                                               6, 7, 4,

                                               2, 3, 7,
                                               7, 6, 2,

                                               1, 2, 6,
                                               6, 5, 1,

                                               0, 3, 7,
                                               7, 4, 0}});
    TextureManager::instance()->registerTexture("big_floppa_diffuse", "./textures/floppa.jpg");
    TextureManager::instance()->registerTexture("big_floppa_emission", "./textures/floppa_emission.jpg");
    TextureManager::instance()->registerTexture("tex_specular", "./textures/specular.png");

    Material floppaCubeMaterial{.diffTextureName = "big_floppa_diffuse", .specTextureSampler = "tex_specular", .emissionTextureSampler = "big_floppa_emission"};

    PrimitiveObject mainCube{.objMesh = "simple_cube", .objMaterial = floppaCubeMaterial, .scale = glm::vec3(1.0f, 1.0f, 1.0f), .rotation = glm::identity<glm::mat4>(), .position = glm::vec3(0.0f, 0.0f, 0.0f)};
    PrimitiveObject lightCube{.objMesh = "simple_cube", .scale = glm::vec3(0.5f, 0.5f, 0.5f)};

    MeshManager::instance()->allocateMesh("simple_cube");
    TextureManager::instance()->allocateTexture("tex_specular");
    TextureManager::instance()->allocateTexture("big_floppa_emission");
    TextureManager::instance()->allocateTexture("big_floppa_diffuse");

    // //// Dummy axes VAO
    // uint dummyVao;
    // glGenVertexArrays(1, &dummyVao);
    // glBindVertexArray(0);

    {
        //// Shaders

        Shader shaderProgramMain{vertexShaderSource, fragmentShaderSource};
        Shader lightCubeShader{lightVertexShaderSource, lightFragmentShaderSource};
        // Shader worldAxesShader{axesVertexShaderSource, axesFragmentShaderSource, axesGeometryShaderSource};

        //// Render loop
        camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
        while (!glfwWindowShouldClose(mainWindow))
        {
            deltaTime = glfwGetTime() - previousTime;
            previousTime = glfwGetTime();

            processInput(mainWindow);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            const float time = glfwGetTime();

            const float lightPosX = std::cos(time * 2.0f) * lightRotationRadius;
            const float lightPosY = std::sin(time * 2.0f) * lightRotationRadius;
            const float lightPosZ = std::sin(time * 3.0f);
            const glm::vec3 normalizedLightPos = glm::normalize(glm::vec3(lightPosX, lightPosY, lightPosZ));

            const glm::mat4 projection = glm::perspective(glm::radians(camera.zoom()), (float)windowWidth / windowHeight, 0.1f, 1000.0f);
            const glm::mat4 view = camera.getViewMatrix();

            // {
            //     glBindVertexArray(dummyVao);

            //     worldAxesShader.use();

            //     worldAxesShader.setMatrix4("viewMat", view);
            //     worldAxesShader.setMatrix4("clipMat", projection);
            //     // TODO: add proper rescaling of axes due to zoom
            //     worldAxesShader.setFloat("axisLength", 0.03l);
            //     worldAxesShader.setFloat("thickness", 0.0005l);
            //     worldAxesShader.setFloat("cameraDistance", glm::length(camera.position()));
            //     worldAxesShader.setFloat("cameraNear", 0.1f);

            //     glDrawArrays(GL_POINTS, 0, 3);

            //     glBindVertexArray(0);
            // }
            {
                shaderProgramMain.use();

                MeshManager::instance()->bindMesh(mainCube.objMesh);

                shaderProgramMain.setInt("currentMaterial.diffTextureSampler", TextureManager::instance()->bindTexture(mainCube.objMaterial.diffTextureName));
                shaderProgramMain.setInt("currentMaterial.specTextureSampler", TextureManager::instance()->bindTexture(mainCube.objMaterial.specTextureSampler));
                shaderProgramMain.setInt("currentMaterial.emissionTextureSampler", TextureManager::instance()->bindTexture(mainCube.objMaterial.emissionTextureSampler));

                glm::mat4 model(1.0f);
                model = glm::scale(model, mainCube.scale);
                model = mainCube.rotation * model;
                model = glm::translate(model, mainCube.position);

                shaderProgramMain.setVec3("currentLight.lightPos", {lightPosX, lightPosY, lightPosZ});
                shaderProgramMain.setVec3("currentLight.specStrength", {0.8f, 0.8f, 0.8f});
                shaderProgramMain.setVec3("currentLight.diffStrength", normalizedLightPos);
                shaderProgramMain.setVec3("currentLight.ambStrength", {0.15f, 0.15f, 0.15f});
                shaderProgramMain.setFloat("currentLight.k", 1.2f);
                shaderProgramMain.setFloat("currentLight.b", 0.1f);

                shaderProgramMain.setVec3("viewPos", camera.position());

                shaderProgramMain.setMatrix4("model", model);
                shaderProgramMain.setMatrix4("view", view);
                shaderProgramMain.setMatrix4("projection", projection);

                glDrawElements(GL_TRIANGLES, MeshManager::instance()->getMesh(mainCube.objMesh)->numIndices(), GL_UNSIGNED_INT, 0);
                MeshManager::instance()->unbindMesh();
                TextureManager::instance()->unbindAllTextures();
            }

            {
                lightCubeShader.use();

                MeshManager::instance()->bindMesh("simple_cube");

                glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), glm::vec3(lightPosX, lightPosY, lightPosZ));
                lightModel = glm::scale(lightModel, glm::vec3(0.6f, 0.6f, 0.6f));

                lightCubeShader.setVec3("actualLightColor", normalizedLightPos);
                lightCubeShader.setMatrix4("model", lightModel);
                lightCubeShader.setMatrix4("view", view);
                lightCubeShader.setMatrix4("projection", projection);

                glDrawElements(GL_TRIANGLES, MeshManager::instance()->getMesh(mainCube.objMesh)->numIndices(), GL_UNSIGNED_INT, 0);
                MeshManager::instance()->unbindMesh();
            }

            glfwSwapBuffers(mainWindow);

            glfwPollEvents();
        }
    }
    //// Cleanup
    MeshManager::instance()->cleanUpGracefully();
    TextureManager::instance()->cleanUpGracefully();

    glfwTerminate();

    return 0;
}
