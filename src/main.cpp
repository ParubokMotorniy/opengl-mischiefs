// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "basicshader.h"
#include "camera.h"
#include "geometryshaderprogram.h"
#include "instancedshader.h"
#include "instancer.h"
#include "lightmanager.h"
#include "lightvisualizationshader.h"
#include "materialmanager.h"
#include "meshmanager.h"
#include "modelloader.h"
#include "object.h"
#include "objectmanager.h"
#include "quaternioncamera.h"
#include "texture.h"
#include "texturemanager.h"
#include "transformmanager.h"
#include "window.h"
#include "worldplaneshader.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <numbers>
#include <tuple>

namespace
{
constexpr size_t windowWidth = 1440;
constexpr size_t windowHeight = 810;

// TODO: add some shader source manager
const char *vertexShaderSource = ENGINE_SHADERS "/vertex_standard.vs";
const char *fragmentShaderSource = ENGINE_SHADERS "/fragment_standard.fs";

const char *axesVertexShaderSource = ENGINE_SHADERS "/axis_vertex.vs";
const char *axesFragmentShaderSource = ENGINE_SHADERS "/axis_fragment.fs";
const char *axesGeometryShaderSource = ENGINE_SHADERS "/axis_geometry.gs";

Camera *camera = new QuaternionCamera(glm::vec3(10.f, 10.0f, -10.0f));
float deltaTime{ 0.0f };
float previousTime{ 0.0f };
const float lightRotationRadius = 70.0f;
bool renderAxes = true;
} // namespace

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
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    //// Meshes
    // TODO: make sure the cubes reuse the same mesh but index vertices in their cutsom way

    const MeshIdentifier simpleCubeMesh = MeshManager::instance()->registerMesh(
        Mesh{ { { -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -5.0f, -5.0f, -5.0f },
                { -0.5f, -0.5f, 0.5f, 1.0f, 1.0f, -5.0f, -5.0f, 5.0f },
                { -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -5.0f, 5.0f, 5.0f },
                { -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -5.0f, 5.0f, -5.0f },

                { 0.5f, -0.5f, -0.5f, 1.0, 1.0f, 5.0f, -5.0f, -5.0f },
                { 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 5.0f, -5.0f, 5.0f },
                { 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 5.0f, 5.0f, 5.0f },
                { 0.5f, 0.5f, -0.5f, 1.00f, 0.0f, 5.0f, 5.0f, -5.0f } },

              { 0, 1, 2, 2, 3, 0,

                0, 4, 5, 5, 1, 0,

                6, 5, 4, 4, 7, 6,

                7, 3, 2, 2, 6, 7,

                6, 2, 1, 1, 5, 6,

                0, 3, 7, 7, 4, 0 } },
        "simple_cube");

    //   0  4
    // 1 3 5 7
    // 2   6

    const MeshIdentifier simplePyramidMesh = MeshManager::instance()->registerMesh(
        Mesh{ { { std::cos(0), 0.0f, std::sin(0), 0.0f, 1.0f, std::cos(0) - std::numbers::sqrt2 / 2,
                  0.0f, std::sin(0) - std::numbers::sqrt2 / 2 },
                { std::cos(2 * std::numbers::pi / 3), 0.0f, std::sin(2 * std::numbers::pi / 3),
                  1.0f, 1.0f, std::cos(2 * std::numbers::pi / 3) - std::numbers::sqrt2 / 2, 0.0f,
                  std::sin(2 * std::numbers::pi / 3) - std::numbers::sqrt2 / 2 },
                { std::cos(4 * std::numbers::pi / 3), 0.0f, std::sin(4 * std::numbers::pi / 3),
                  1.0f, 0.0f, std::cos(4 * std::numbers::pi / 3) - std::numbers::sqrt2 / 2, 0.0f,
                  std::sin(4 * std::numbers::pi / 3) - std::numbers::sqrt2 / 2 },
                { 0.0f, std::numbers::sqrt2, 0.0f, 0.0f, 0.0f, 0.0f, std::numbers::sqrt2, 0.0f } },

              { 0, 1, 2, 1, 3, 2, 2, 3, 0, 0, 3, 1 } },
        "simple_pyramid");

    const MeshIdentifier dummyAxesMesh = MeshManager::instance()->registerMesh(Mesh(),
                                                                               "dummy_mesh");

    // Textures

    const TextureIdentifier floppaDiff = TextureManager::instance()
                                             ->registerTexture(ENGINE_TEXTURES "/floppa.jpg",
                                                               "big_floppa_diffuse");

    const TextureIdentifier catDiff = TextureManager::instance()->registerTexture(ENGINE_TEXTURES
                                                                                  "/silly_cat.jpg",
                                                                                  "cat_diffuse");
    const TextureIdentifier black = TextureManager::instance()->registerTexture(ENGINE_TEXTURES
                                                                                "/black.jpg",
                                                                                "black");

    const TextureIdentifier specular = TextureManager::instance()->registerTexture(ENGINE_TEXTURES
                                                                                   "/specular.png",
                                                                                   "tex_specular");
    const TextureIdentifier floppaEm = TextureManager::instance()
                                           ->registerTexture(ENGINE_TEXTURES "/floppa_emission.jpg",
                                                             "big_floppa_emission");

    const TextureIdentifier checkerboardTexture
        = TextureManager::instance()->registerTexture(ENGINE_TEXTURES "/checkerboard_pattern.jpg",
                                                      "checkerboard");
    {
        auto checkerTexture = TextureManager::instance()->getTexture(checkerboardTexture);
        checkerTexture->setUseAnisotropic(true, 8);
        checkerTexture->setParameters(Texture2DParameters{ .wrappingS = GL_MIRRORED_REPEAT,
                                                           .wrappingT = GL_MIRRORED_REPEAT,
                                                           .filteringMin = GL_LINEAR_MIPMAP_LINEAR,
                                                           .filteringMag = GL_LINEAR });
    }

    // Materials
    const MaterialIdentifier floppaMaterial
        = MaterialManager<BasicMaterial, ComponentType::BASIC_MATERIAL>::instance()
              ->registerMaterial(BasicMaterial{ floppaDiff, specular, floppaEm },
                                 "floppa_material");

    const MaterialIdentifier catMaterial
        = MaterialManager<BasicMaterial, ComponentType::BASIC_MATERIAL>::instance()
              ->registerMaterial(BasicMaterial{ catDiff, specular, black }, "cat_material");

    const MaterialIdentifier checkerMaterial
        = MaterialManager<BasicMaterial, ComponentType::BASIC_MATERIAL>::instance()
              ->registerMaterial(BasicMaterial{ checkerboardTexture, InvalidIdentifier,
                                                InvalidIdentifier },
                                 "checker_material");

    // Models

    // Attribution: Bill Cipher 3D by Coolguy5SuperDuperCool from sketchfab
    const GameObjectIdentifier billModel = ModelLoader::instance()->loadModel(
        ENGINE_MODELS
        "/bill/bill_cipher.obj"); // put "/tank/tank.obj" here to test a different model.
                                  // Surprisingly, it seems to be better optimized than bill

    //"Sphere" (https://skfb.ly/CR77) by oatmas64134 is licensed under CC
    //Attribution-NonCommercial-ShareAlike (http://creativecommons.org/licenses/by-nc-sa/4.0/).
    const GameObjectIdentifier sphereModel = ModelLoader::instance()->loadModel(
        ENGINE_MODELS "/sphere/sphere.obj");
    const MeshIdentifier sphereMesh = MeshManager::instance()->meshRegistered("Sphere");

    // Events
    {
        mainWindow.subscribeEventListener(
            [&](KeyboardInput input, KeyboardInput releasedKeys, float deltaTime) {
                if (releasedKeys.CtrlLeft)
                    renderAxes = !renderAxes;
            });
        mainWindow.subscribeEventListener(
            [camPtr = camera](KeyboardInput input, KeyboardInput releasedKeys, float deltaTime) {
                camPtr->processKeyboard(input, deltaTime);
            });
        mainWindow.subscribeEventListener(
            [camPtr = camera, &mainWindow](KeyboardInput input,
                                           Window::MouseMotionDescriptor descriptor) {
                if (input.MouseRight == 1)
                {
                    camPtr->processMouseMovement(descriptor.deltaPosX, descriptor.deltaPosY);
                    mainWindow.hideCursor(true);
                    mainWindow.setMouseAccuracy(true);
                }
                else
                {
                    mainWindow.hideCursor(false);
                    mainWindow.setMouseAccuracy(false);
                }
            });
        mainWindow.subscribeEventListener(
            [camPtr = camera](KeyboardInput input, Window::ScrollDescriptor descriptor) {
                camPtr->processMouseScroll(descriptor.deltaScrollY);
            });
        mainWindow.subscribeEventListener([](int w, int h) { glViewport(0, 0, w, h); });
    }

    {
        //// Shaders

        InstancedShader shaderProgramMain{ vertexShaderSource, fragmentShaderSource };
        shaderProgramMain.initializeShaderProgram();

        GeometryShaderProgram worldAxesShader{ axesVertexShaderSource, axesFragmentShaderSource,
                                               axesGeometryShaderSource, dummyAxesMesh };
        worldAxesShader.initializeShaderProgram();

        WorldPlaneShader worldPlaneShader{ simpleCubeMesh, checkerboardTexture };
        worldPlaneShader.initializeShaderProgram();

        LightVisualizationShader lightVisualizationShader{ sphereMesh };
        lightVisualizationShader.initializeShaderProgram();

        // shader for rendering of cubes in non-instanced way
        // BasicShader cubeScatterer{ simpleCubeMesh, checkerboardTexture, 1000, 1000 };
        // cubeScatterer.initializeShaderProgram();

        {
            {
                GameObject &pointLight1 = ObjectManager::instance()->getObject(
                    ObjectManager::instance()->addObject());
                const auto lightTransform = TransformManager::instance()->registerNewTransform(
                    pointLight1);
                const LightSourceIdentifier lId
                    = LightManager<ComponentType::LIGHT_POINT>::instance()
                          ->registerNewLight("test_light_1", lightTransform);
                pointLight1.addComponent(Component(ComponentType::LIGHT_POINT, lId));
                pointLight1.addComponent(Component(ComponentType::TRANSFORM, lightTransform));
                auto lightStruct = LightManager<ComponentType::LIGHT_POINT>::instance()->getLight(
                    lId);
                lightStruct->ambient = glm::vec3(0.867f, 0.922f, 0.518f);
                lightStruct->diffuse = glm::vec3(0.792f, 0.98f, 0.933f);
                lightStruct->specular = glm::vec3(0.882f, 0.969f, 0.949f);

                auto transformStruct = TransformManager::instance()->getTransform(lightTransform);
                transformStruct->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
                transformStruct->setPosition(glm::vec3(-15.0f, -15.0f, -15.0f));

                lightVisualizationShader.addObject(pointLight1);
            }

            {
                GameObject &pointLight2 = ObjectManager::instance()->getObject(
                    ObjectManager::instance()->addObject());
                const auto lightTransform = TransformManager::instance()->registerNewTransform(
                    pointLight2);
                const LightSourceIdentifier lId
                    = LightManager<ComponentType::LIGHT_POINT>::instance()
                          ->registerNewLight("test_light_2", lightTransform);
                pointLight2.addComponent(Component(ComponentType::LIGHT_POINT, lId));
                pointLight2.addComponent(Component(ComponentType::TRANSFORM, lightTransform));

                auto lightStruct = LightManager<ComponentType::LIGHT_POINT>::instance()->getLight(
                    lId);
                lightStruct->ambient = glm::vec3(0.89f, 0.439f, 0.369f);
                lightStruct->diffuse = glm::vec3(0.969f, 0.545f, 0.71f);
                lightStruct->specular = glm::vec3(0.98f, 0.702f, 0.808f);

                auto transformStruct = TransformManager::instance()->getTransform(lightTransform);
                transformStruct->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
                transformStruct->setPosition(glm::vec3(15.0f, 15.0f, 15.0f));

                lightVisualizationShader.addObject(pointLight2);
            }
        }

        for (int k = 5; k < 65; ++k)
        {
            // generates lots of instanced cubes
            //  for (int h = 0; h < 1000; ++h)
            //  {
            //      GameObject &standardObject = ObjectManager::instance()->getObject(
            //          ObjectManager::instance()->addObject());

            //     standardObject.addComponent(Component(ComponentType::MESH, simpleCubeMesh));
            //     standardObject.addComponent(
            //         Component(ComponentType::BASIC_MATERIAL, checkerMaterial));

            //     const TransformIdentifier tId =
            //     TransformManager::instance()->registerNewTransform(
            //         standardObject);
            //     standardObject.addComponent(Component(ComponentType::TRANSFORM, tId));

            //     Transform *t = TransformManager::instance()->getTransform(tId);
            //     t->setPosition(glm::vec3((k - 500) * 7, 0.0f, (h - 500) * 7));
            //     t->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
            //     shaderProgramMain.addObject(standardObject);
            // }

            // add cubes and pyramids
            GameObject &standardObject = ObjectManager::instance()->getObject(
                ObjectManager::instance()->addObject());

            standardObject.addComponent(
                Component(ComponentType::MESH, (k % 20 < 10) ? simpleCubeMesh : simplePyramidMesh));
            standardObject.addComponent(Component(ComponentType::BASIC_MATERIAL,
                                                  (k % 20 < 10) ? floppaMaterial : catMaterial));

            const TransformIdentifier tId = TransformManager::instance()->registerNewTransform(
                standardObject);
            standardObject.addComponent(Component(ComponentType::TRANSFORM, tId));

            Transform *t = TransformManager::instance()->getTransform(tId);
            t->setPosition(glm::vec3(k * std::sin(k), -k * std::sin(k), k * std::cos(k)));
            t->setRotation(glm::rotate(t->rotation(), glm::radians((float)k),
                                       glm::vec3(50.0f - k, 1.0f, 0.0f)));
            t->setScale(glm::vec3(std::max((k % 10) / 2.0f, 0.2f)));

            shaderProgramMain.addObject(standardObject);

            /// add axes for cubes and pyramids
            GameObject &standardAxes = ObjectManager::instance()->getObject(
                ObjectManager::instance()->addObject());
            standardAxes.addComponent(Component(ComponentType::MESH, dummyAxesMesh));
            standardAxes.addComponent(Component(ComponentType::TRANSFORM, tId));

            worldAxesShader.addObject(standardAxes);

            /// add bills
            auto billCopy = ObjectManager::instance()->copyObject(billModel);
            auto billTransform = TransformManager::instance()->getTransform(
                ObjectManager::instance()->getObject(billCopy).getIdentifierForComponent(
                    ComponentType::TRANSFORM));
            billTransform->setPosition(
                glm::vec3(-k * std::sin(k), k * std::sin(k), k * std::cos(k)));
            billTransform->setRotation(glm::rotate(billTransform->rotation(),
                                                   glm::radians((float)k),
                                                   glm::vec3(50.0f - k, 1.0f, 0.0f)));
            billTransform->setScale(glm::vec3(20.0f));

            shaderProgramMain.addObjectWithChildren(billCopy);
        }
        {
            GameObject &worldAxes = ObjectManager::instance()->getObject(
                ObjectManager::instance()->addObject());
            worldAxes.addComponent(Component(ComponentType::MESH, dummyAxesMesh));
            worldAxes.addComponent(
                Component(ComponentType::TRANSFORM,
                          TransformManager::instance()->registerNewTransform(worldAxes)));
            worldAxesShader.addObject(worldAxes);
        }
        {
            TransformManager::instance()->flushUpdates();
            shaderProgramMain.runTextureMapping();
            shaderProgramMain.runInstancing();

            //// Render loop
            camera->moveTo(glm::vec3(0.0f, 7.0f, 0.0f));
            camera->lookAt(glm::vec3(-10.0f, 7.0f, -1.0f));
        }
        while (!mainWindow.shouldClose())
        {
            // TODO: move time management to a separate class
            const float time = glfwGetTime();
            deltaTime = time - previousTime;
            previousTime = time;

            mainWindow.update(deltaTime);

            const float lightPosX = std::cos(time * 1.5f) * lightRotationRadius;
            const float lightPosY = std::sin(time * 2.0f) * lightRotationRadius;
            const float lightPosZ = std::sin(time * 2.5f) * lightRotationRadius;
            const glm::vec3 normalizedLightPos = glm::normalize(
                glm::vec3(lightPosX, lightPosY, lightPosZ));

            const glm::mat4 projection = glm::perspective(glm::radians(camera->zoom()),
                                                          (float)windowWidth / windowHeight, 0.1f,
                                                          1000.0f);
            const glm::mat4 view = camera->getViewMatrix();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            {
                shaderProgramMain.use();

                shaderProgramMain.setVec3("currentLight.lightPos",
                                          { lightPosX, lightPosY,
                                            lightPosZ }); // TODO: generalize for multiple sources
                shaderProgramMain.setVec3("currentLight.specStrength", { 0.8f, 0.8f, 0.8f });
                shaderProgramMain.setVec3("currentLight.diffStrength", normalizedLightPos);
                shaderProgramMain.setVec3("currentLight.ambStrength", { 0.15f, 0.15f, 0.15f });
                shaderProgramMain.setFloat("currentLight.k", 1.2f);
                shaderProgramMain.setFloat("currentLight.b", 0.1f);

                shaderProgramMain.setMatrix4("view", view);
                shaderProgramMain.setMatrix4("projection", projection);

                shaderProgramMain.setVec3("viewPos", camera->position());

                shaderProgramMain.runShader();
            }

            {
                worldPlaneShader.use();

                worldPlaneShader.setMatrix4("view", view);
                worldPlaneShader.setMatrix4("projection", projection);

                worldPlaneShader.setFloat("checkerUnitWidth", 5.0f);
                worldPlaneShader.setFloat("checkerUnitHeight", 5.0f);

                worldPlaneShader.runShader();
            }
            {
                lightVisualizationShader.use();
                lightVisualizationShader.setMatrix4("view", view);
                lightVisualizationShader.setMatrix4("projection", projection);

                lightVisualizationShader.runShader();
            }

            // // renders lots of cubes in non-instanced way
            // {
            //     cubeScatterer.use();
            //     cubeScatterer.setMatrix4("view", view);
            //     cubeScatterer.setMatrix4("projection", projection);
            //     cubeScatterer.runShader();
            // }

            if (renderAxes)
            {
                glDisable(GL_DEPTH_TEST);
                worldAxesShader.use();

                worldAxesShader.setFloat("axisLength", 0.25l);
                worldAxesShader.setFloat("thickness", 0.002l);
                worldAxesShader.setMatrix4("viewMat", view);
                worldAxesShader.setMatrix4("projectionMat", projection);

                worldAxesShader.runShader();

                glEnable(GL_DEPTH_TEST);
            }

            {
                glfwSwapBuffers(mainWindow.getRawWindow());
                TransformManager::instance()->flushUpdates();
                glfwPollEvents();
            }
        }
    }
    //// Cleanup
    MeshManager::instance()->cleanUpGracefully();
    TextureManager::instance()->cleanUpGracefully();

    glfwTerminate();

    return 0;
}
