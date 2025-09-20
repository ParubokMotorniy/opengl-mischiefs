#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaderprogram.h"
#include "geometryshaderprogram.h"
#include "camera.h"
#include "quaternioncamera.h"
#include "texture.h"
#include "meshmanager.h"
#include "texturemanager.h"
#include "object.h"
#include "window.h"

#include <iostream>
#include <cmath>
#include <tuple>

namespace
{
    constexpr size_t windowWidth = 1440;
    constexpr size_t windowHeight = 810;

    // TODO: add some shader source manager
    const char *vertexShaderSource = "./shaders/vertex_standard.vs";
    const char *fragmentShaderSource = "./shaders/fragment_standard.fs";

    const char *lightVertexShaderSource = "./shaders/light_vertex.vs";
    const char *lightFragmentShaderSource = "./shaders/light_fragment.fs";

    const char *axesVertexShaderSource = "./shaders/axis_vertex.vs";
    const char *axesFragmentShaderSource = "./shaders/axis_fragment.fs";
    const char *axesGeometryShaderSource = "./shaders/axis_geometry.gs";

    const char *voronoiDistanceFragmentShaderSource = "./shaders/voronoi_distances.fs";
    const char *voronoiseFragmentShaderSource = "./shaders/voronoise.fs";

    Camera *camera = new QuaternionCamera(glm::vec3(10.f, 10.0f, -10.0f));
    float deltaTime{0.0f};
    float previousTime{0.0f};
    const float lightRotationRadius = 40.0f;
    bool renderAxes = true;
}

MeshManager *MeshManager::_instance = nullptr;
TextureManager *TextureManager::_instance = nullptr;

int main(int argc, const char *argv[])
{
    glfwInit();
    Window mainWindow(windowWidth, windowHeight, "opengl-mischiefs");

    if (mainWindow.getRawWindow() == nullptr)
    {
        std::cerr << "Failed to create a main window!" << std::endl;
        return -1;
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
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
                                              {{-5.0f, -5.0f, -5.0f, 0.0f, 1.0f, -5.0f, -5.0f, -5.0f},
                                               {-5.0f, -5.0f, 5.0f, 1.0f, 1.0f, -5.0f, -5.0f, 5.0f},
                                               {-5.0f, 5.0f, 5.0f, 1.0f, 0.0f, -5.0f, 5.0f, 5.0f},
                                               {-5.0f, 5.0f, -5.0f, 0.0f, 0.0f, -5.0f, 5.0f, -5.0f},

                                               {5.0f, -5.0f, -5.0f, 1.0, 1.0f, 5.0f, -5.0f, -5.0f},
                                               {5.0f, -5.0f, 5.0f, 0.0f, 1.0f, 5.0f, -5.0f, 5.0f},
                                               {5.0f, 5.0f, 5.0f, 0.0f, 0.0f, 5.0f, 5.0f, 5.0f},
                                               {5.0f, 5.0f, -5.0f, 1.00f, 0.0f, 5.0f, 5.0f, -5.0f}},

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

    //   0  4
    // 1 3 5 7
    // 2   6
    MeshManager::instance()->registerMesh("half_cube_up", Mesh{
                                                              {{-5.0f, -5.0f, -5.0f, 0.0f, 1.0f, -5.0f, -5.0f, -5.0f},
                                                               {-5.0f, -5.0f, 5.0f, 1.0f, 1.0f, -5.0f, -5.0f, 5.0f},
                                                               {-5.0f, 5.0f, 5.0f, 1.0f, 0.0f, -5.0f, 5.0f, 5.0f},
                                                               {-5.0f, 5.0f, -5.0f, 0.0f, 0.0f, -5.0f, 5.0f, -5.0f},

                                                               {5.0f, -5.0f, -5.0f, 1.0, 1.0f, 5.0f, -5.0f, -5.0f},
                                                               {5.0f, -5.0f, 5.0f, 0.0f, 1.0f, 5.0f, -5.0f, 5.0f},
                                                               {5.0f, 5.0f, 5.0f, 0.0f, 0.0f, 5.0f, 5.0f, 5.0f},
                                                               {5.0f, 5.0f, -5.0f, 1.00f, 0.0f, 5.0f, 5.0f, -5.0f}},

                                                              {3, 0, 1,
                                                               2, 1, 5,
                                                               6, 5, 4,
                                                               7, 4, 0,
                                                               4, 5, 1,
                                                               3, 2, 6}});

    MeshManager::instance()->registerMesh("half_cube_down", Mesh{
                                                                {{-5.0f, -5.0f, -5.0f, 0.0f, 1.0f, -5.0f, -5.0f, -5.0f},
                                                                 {-5.0f, -5.0f, 5.0f, 1.0f, 1.0f, -5.0f, -5.0f, 5.0f},
                                                                 {-5.0f, 5.0f, 5.0f, 1.0f, 0.0f, -5.0f, 5.0f, 5.0f},
                                                                 {-5.0f, 5.0f, -5.0f, 0.0f, 0.0f, -5.0f, 5.0f, -5.0f},

                                                                 {5.0f, -5.0f, -5.0f, 1.0, 1.0f, 5.0f, -5.0f, -5.0f},
                                                                 {5.0f, -5.0f, 5.0f, 0.0f, 1.0f, 5.0f, -5.0f, 5.0f},
                                                                 {5.0f, 5.0f, 5.0f, 0.0f, 0.0f, 5.0f, 5.0f, 5.0f},
                                                                 {5.0f, 5.0f, -5.0f, 1.00f, 0.0f, 5.0f, 5.0f, -5.0f}},

                                                                {1, 2, 3,
                                                                 5, 6, 2,
                                                                 4, 7, 6,
                                                                 0, 3, 7,
                                                                 1, 0, 4,
                                                                 6, 7, 3}});

    TextureManager::instance()->registerTexture("big_floppa_diffuse", "./textures/floppa.jpg");
    TextureManager::instance()->registerTexture("big_floppa_emission", "./textures/floppa_emission.jpg");
    TextureManager::instance()->registerTexture("tex_specular", "./textures/specular.png");

    Material floppaCubeMaterial{.diffTextureName = "big_floppa_diffuse", .specTextureSampler = "tex_specular", .emissionTextureSampler = "big_floppa_emission"};

    MeshManager::instance()->allocateMesh("simple_cube");
    MeshManager::instance()->allocateMesh("half_cube_up");
    MeshManager::instance()->allocateMesh("half_cube_down");
    TextureManager::instance()->allocateTexture("tex_specular");
    TextureManager::instance()->allocateTexture("big_floppa_emission");
    TextureManager::instance()->allocateTexture("big_floppa_diffuse");

    std::vector<PrimitiveObject> standardShaderObjects = {{.objMesh = "simple_cube", .objMaterial = floppaCubeMaterial, .rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 1.0f)), .position = glm::vec3(-10.0f, 10.0f, -10.0f)}};
    std::vector<PrimitiveObject> cubeLightObjects = {{.objMesh = "simple_cube", .scale = glm::vec3(0.5f, 0.5f, 0.5f)}};
    std::vector<PrimitiveObject> voronoiseObjects = {{.objMesh = "half_cube_down", .scale = glm::vec3(1.0f, 1.0f, 1.0f), .rotation = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 1.0f)), .position = glm::vec3(10.0f, -10.0f, 10.0f)}};
    std::vector<PrimitiveObject> voronoiDistancesObjects = {{.objMesh = "half_cube_up", .scale = glm::vec3(1.0f, 1.0f, 1.0f), .rotation = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 1.0f)), .position = glm::vec3(10.0f, -10.0f, 10.0f)}};
    std::vector<PrimitiveObject *> objectsWithAxes = {&standardShaderObjects[0], &voronoiseObjects[0], &voronoiDistancesObjects[0]};

    mainWindow.subscribeEventListener([&](KeyboardInput input, KeyboardInput releasedKeys, float deltaTime)
                                      { 
                                        if(releasedKeys.CtrlLeft)
                                            renderAxes = !renderAxes; });
    mainWindow.subscribeEventListener([camPtr = camera](KeyboardInput input, KeyboardInput releasedKeys, float deltaTime)
                                      { camPtr->processKeyboard(input, deltaTime); });
    mainWindow.subscribeEventListener([camPtr = camera](KeyboardInput input, Window::MouseMotionDescriptor descriptor)
                                      { 
                                        if(input.MouseRight == 1)
                                        {
                                            camPtr->processMouseMovement(descriptor.deltaPosX, descriptor.deltaPosY);
                                        } });
    mainWindow.subscribeEventListener([camPtr = camera](KeyboardInput input, Window::ScrollDescriptor descriptor)
                                      { camPtr->processMouseScroll(descriptor.deltaScrollY); });
    mainWindow.subscribeEventListener([](int w, int h)
                                      { glViewport(0, 0, w, h); });

    // //// Dummy axes VAO
    uint dummyVao;
    glGenVertexArrays(1, &dummyVao);
    glBindVertexArray(0);

    {
        //// Shaders

        ShaderProgram shaderProgramMain{vertexShaderSource, fragmentShaderSource};
        shaderProgramMain.initializeShaderProgram();

        ShaderProgram lightCubeShader{lightVertexShaderSource, lightFragmentShaderSource};
        lightCubeShader.initializeShaderProgram();

        ShaderProgram voronoiseShader{vertexShaderSource, voronoiseFragmentShaderSource};
        voronoiseShader.initializeShaderProgram();

        ShaderProgram voronoiDistancesShader{vertexShaderSource, voronoiDistanceFragmentShaderSource};
        voronoiDistancesShader.initializeShaderProgram();

        GeometryShaderProgram worldAxesShader{axesVertexShaderSource, axesFragmentShaderSource, axesGeometryShaderSource};
        worldAxesShader.initializeShaderProgram();

        //// Render loop
        camera->lookAt(glm::vec3(-10.0f, 10.0f, -10.0f));
        while (!mainWindow.shouldClose())
        {
            // TODO: move time management to a separate class
            deltaTime = glfwGetTime() - previousTime;
            previousTime = glfwGetTime();

            mainWindow.update(deltaTime);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            const float time = glfwGetTime();

            const float lightPosX = std::cos(time * 1.5f) * lightRotationRadius;
            const float lightPosY = std::sin(time * 2.0f) * lightRotationRadius;
            const float lightPosZ = std::sin(time * 2.5f) * lightRotationRadius;
            const glm::vec3 normalizedLightPos = glm::normalize(glm::vec3(lightPosX, lightPosY, lightPosZ));

            const glm::mat4 projection = glm::perspective(glm::radians(camera->zoom()), (float)windowWidth / windowHeight, 0.1f, 1000.0f);
            const glm::mat4 view = camera->getViewMatrix();

            {
                shaderProgramMain.use();

                shaderProgramMain.setVec3("currentLight.lightPos", {lightPosX, lightPosY, lightPosZ}); // TODO: generalize for multiple sources
                shaderProgramMain.setVec3("currentLight.specStrength", {0.8f, 0.8f, 0.8f});
                shaderProgramMain.setVec3("currentLight.diffStrength", normalizedLightPos);
                shaderProgramMain.setVec3("currentLight.ambStrength", {0.15f, 0.15f, 0.15f});
                shaderProgramMain.setFloat("currentLight.k", 1.2f);
                shaderProgramMain.setFloat("currentLight.b", 0.1f);

                shaderProgramMain.setMatrix4("view", view);
                shaderProgramMain.setMatrix4("projection", projection);

                shaderProgramMain.setVec3("viewPos", camera->position());

                for (const PrimitiveObject &obj : standardShaderObjects)
                {
                    MeshManager::instance()->bindMesh(obj.objMesh);

                    shaderProgramMain.setInt("currentMaterial.diffTextureSampler", TextureManager::instance()->bindTexture(standardShaderObjects[0].objMaterial.diffTextureName));
                    shaderProgramMain.setInt("currentMaterial.specTextureSampler", TextureManager::instance()->bindTexture(standardShaderObjects[0].objMaterial.specTextureSampler));
                    shaderProgramMain.setInt("currentMaterial.emissionTextureSampler", TextureManager::instance()->bindTexture(standardShaderObjects[0].objMaterial.emissionTextureSampler));

                    shaderProgramMain.setMatrix4("model", obj.computeModelMatrix());

                    glDrawElements(GL_TRIANGLES, MeshManager::instance()->getMesh(obj.objMesh)->numIndices(), GL_UNSIGNED_INT, 0);
                    MeshManager::instance()->unbindMesh();
                }

                TextureManager::instance()->unbindAllTextures();
            }

            {
                voronoiseShader.use();
                voronoiseShader.setVec3("iResolution", glm::vec3(windowWidth, windowHeight, 0.0f));
                voronoiseShader.setFloat("iTime", time);
                voronoiseShader.setMatrix4("view", view);
                voronoiseShader.setMatrix4("projection", projection);

                for (const PrimitiveObject &obj : voronoiseObjects)
                {
                    MeshManager::instance()->bindMesh(obj.objMesh);

                    voronoiseShader.setMatrix4("model", obj.computeModelMatrix());

                    glDrawElements(GL_TRIANGLES, MeshManager::instance()->getMesh(obj.objMesh)->numIndices(), GL_UNSIGNED_INT, 0);
                    MeshManager::instance()->unbindMesh();
                }
            }

            {
                voronoiDistancesShader.use();
                voronoiDistancesShader.setVec3("iResolution", glm::vec3(1.0f, 1.0f, 0.0f));
                voronoiDistancesShader.setFloat("iTime", time);
                voronoiDistancesShader.setMatrix4("view", view);
                voronoiDistancesShader.setMatrix4("projection", projection);

                for (const PrimitiveObject &obj : voronoiDistancesObjects)
                {
                    MeshManager::instance()->bindMesh(obj.objMesh);

                    voronoiDistancesShader.setMatrix4("model", obj.computeModelMatrix());

                    glDrawElements(GL_TRIANGLES, MeshManager::instance()->getMesh(obj.objMesh)->numIndices(), GL_UNSIGNED_INT, 0);
                    MeshManager::instance()->unbindMesh();
                }

                TextureManager::instance()->unbindAllTextures();
            }

            {
                lightCubeShader.use();

                for (const PrimitiveObject &lightCube : cubeLightObjects)
                {
                    MeshManager::instance()->bindMesh(lightCube.objMesh);
                    glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), glm::vec3(lightPosX, lightPosY, lightPosZ));
                    lightModel = glm::scale(lightModel, glm::vec3(0.6f, 0.6f, 0.6f));

                    lightCubeShader.setVec3("actualLightColor", normalizedLightPos);
                    lightCubeShader.setMatrix4("model", lightModel);
                    lightCubeShader.setMatrix4("view", view);
                    lightCubeShader.setMatrix4("projection", projection);

                    glDrawElements(GL_TRIANGLES, MeshManager::instance()->getMesh(lightCube.objMesh)->numIndices(), GL_UNSIGNED_INT, 0);
                    MeshManager::instance()->unbindMesh();
                }
            }

            if (renderAxes)
            {
                glDisable(GL_DEPTH_TEST);
                worldAxesShader.use();

                glBindVertexArray(dummyVao);

                worldAxesShader.setFloat("axisLength", 0.25l);
                worldAxesShader.setFloat("thickness", 0.002l);
                worldAxesShader.setMatrix4("viewMat", view);
                worldAxesShader.setMatrix4("projectionMat", projection);

                // TODO: do with instancing in the future
                for (const PrimitiveObject *obj : objectsWithAxes)
                {
                    worldAxesShader.setMatrix4("modelMat", obj->computeModelMatrixNoScale());
                    glDrawArrays(GL_POINTS, 0, 3);
                }

                worldAxesShader.setMatrix4("modelMat", glm::identity<glm::mat4>()); // draws global axes
                glDrawArrays(GL_POINTS, 0, 3);

                glBindVertexArray(0);
                glEnable(GL_DEPTH_TEST);
            }

            glfwSwapBuffers(mainWindow.getRawWindow());

            glfwPollEvents();
        }
    }
    //// Cleanup
    MeshManager::instance()->cleanUpGracefully();
    TextureManager::instance()->cleanUpGracefully();

    glfwTerminate();

    return 0;
}
