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
#include <unordered_set>

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
const float lightRotationRadius = 50.0f;

bool renderAxes = true;
bool renderOnlyGrid = false;
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

    const TextureIdentifier spotLightTexture
        = TextureManager::instance()->registerTexture(ENGINE_TEXTURES "/bill.jpg", "bill");

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
    // Attribution-NonCommercial-ShareAlike (http://creativecommons.org/licenses/by-nc-sa/4.0/).
    const GameObjectIdentifier sphereModel = ModelLoader::instance()->loadModel(
        ENGINE_MODELS "/sphere/sphere.obj");
    const MeshIdentifier sphereMesh = MeshManager::instance()->meshRegistered("Sphere");

    // Events
    {
        mainWindow.subscribeEventListener(
            [&](KeyboardInput input, KeyboardInput releasedKeys, float deltaTime) {
                if (releasedKeys.CtrlLeft)
                    renderAxes = !renderAxes;
                if (releasedKeys.CtrlRight)
                    renderOnlyGrid = !renderOnlyGrid;
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

        GameObject &pointLight1 = ObjectManager::instance()->getObject(
            ObjectManager::instance()->addObject());
        const auto pointLight1Transform = TransformManager::instance()->registerNewTransform(
            pointLight1);
        const LightSourceIdentifier pointLight1Light
            = LightManager<ComponentType::LIGHT_POINT>::instance()
                  ->registerNewLight("test_point_light_1", pointLight1Transform);
        {
            pointLight1.addComponent(Component(ComponentType::LIGHT_POINT, pointLight1Light));
            pointLight1.addComponent(Component(ComponentType::TRANSFORM, pointLight1Transform));
            auto lightStruct = LightManager<ComponentType::LIGHT_POINT>::instance()->getLight(
                pointLight1Light);
            lightStruct->ambient = glm::vec3(0.929f, 0.878f, 0.675f);
            lightStruct->diffuse = glm::vec3(0.922f, 0.835f, 0.498f);
            lightStruct->specular = glm::vec3(0.91f, 0.878f, 0.757f);
            lightStruct->attenuationConstantTerm = 1.0e-2;
            lightStruct->attenuationLinearTerm = 1.0e-2;
            lightStruct->attenuationQuadraticTerm = 1.0e-3;

            auto transformStruct = TransformManager::instance()->getTransform(pointLight1Transform);
            transformStruct->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
            transformStruct->setPosition(glm::vec3(-20.0f, -20.0f, -20.0f));

            lightVisualizationShader.addObject(pointLight1);
        }

        GameObject &pointLight2 = ObjectManager::instance()->getObject(
            ObjectManager::instance()->addObject());
        const auto pointLight2Transform = TransformManager::instance()->registerNewTransform(
            pointLight2);
        const LightSourceIdentifier pointLight2Light
            = LightManager<ComponentType::LIGHT_POINT>::instance()
                  ->registerNewLight("test_point_light_2", pointLight2Transform);
        {
            pointLight2.addComponent(Component(ComponentType::LIGHT_POINT, pointLight2Light));
            pointLight2.addComponent(Component(ComponentType::TRANSFORM, pointLight2Transform));

            auto lightStruct = LightManager<ComponentType::LIGHT_POINT>::instance()->getLight(
                pointLight2Light);
            lightStruct->ambient = glm::vec3(0.89f, 0.439f, 0.369f);
            lightStruct->diffuse = glm::vec3(0.969f, 0.545f, 0.71f);
            lightStruct->specular = glm::vec3(0.98f, 0.702f, 0.808f);
            lightStruct->attenuationConstantTerm = 1.0e-2;
            lightStruct->attenuationLinearTerm = 1.0e-2;
            lightStruct->attenuationQuadraticTerm = 1.0e-3;

            auto transformStruct = TransformManager::instance()->getTransform(pointLight2Transform);
            transformStruct->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
            transformStruct->setPosition(glm::vec3(20.0f, 20.0f, 20.0f));

            lightVisualizationShader.addObject(pointLight2);
        }
        {
            {
                GameObject &dirLight1 = ObjectManager::instance()->getObject(
                    ObjectManager::instance()->addObject());
                const auto lightTransform = TransformManager::instance()->registerNewTransform(
                    dirLight1);
                const LightSourceIdentifier lId
                    = LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()
                          ->registerNewLight("test_dir_light_1", lightTransform);
                dirLight1.addComponent(Component(ComponentType::LIGHT_DIRECTIONAL, lId));
                dirLight1.addComponent(Component(ComponentType::TRANSFORM, lightTransform));
                auto lightStruct = LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()
                                       ->getLight(lId);
                lightStruct->ambient = glm::vec3(0.102f, 0.031f, 0.039f);
                lightStruct->diffuse = glm::vec3(0.129f, 0.078f, 0.086f);
                lightStruct->specular = glm::vec3(0.075f, 0.012f, 0.02f);

                auto transformStruct = TransformManager::instance()->getTransform(lightTransform);
                transformStruct->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
                transformStruct->setPosition(glm::vec3(0.0f, 30.0f, 0.0f));
                transformStruct->setRotation(glm::rotate(glm::identity<glm::mat4>(),
                                                         glm::radians(55.0f),
                                                         glm::vec3(1.0f, -1.0f, 0.0f)));

                lightVisualizationShader.addObject(dirLight1);
            }

            {
                GameObject &dirLight2 = ObjectManager::instance()->getObject(
                    ObjectManager::instance()->addObject());
                const auto lightTransform = TransformManager::instance()->registerNewTransform(
                    dirLight2);
                const LightSourceIdentifier lId
                    = LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()
                          ->registerNewLight("test_dir_light_2", lightTransform);
                dirLight2.addComponent(Component(ComponentType::LIGHT_DIRECTIONAL, lId));
                dirLight2.addComponent(Component(ComponentType::TRANSFORM, lightTransform));
                auto lightStruct = LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()
                                       ->getLight(lId);
                lightStruct->ambient = glm::vec3(0.098f, 0.133f, 0.051f);
                lightStruct->diffuse = glm::vec3(0.098f, 0.137f, 0.051f);
                lightStruct->specular = glm::vec3(0.035f, 0.055f, 0.012f);

                auto transformStruct = TransformManager::instance()->getTransform(lightTransform);
                transformStruct->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
                transformStruct->setPosition(glm::vec3(0.0f, -30.0f, 0.0f));
                transformStruct->setRotation(glm::rotate(glm::identity<glm::mat4>(),
                                                         glm::radians(55.0f),
                                                         glm::vec3(-1.0f, 1.0f, 0.0f)));

                lightVisualizationShader.addObject(dirLight2);
            }

            {
                // yellow
                GameObject &spotLight1 = ObjectManager::instance()->getObject(
                    ObjectManager::instance()->addObject());
                const auto lightTransform = TransformManager::instance()->registerNewTransform(
                    spotLight1);
                const LightSourceIdentifier lId
                    = LightManager<ComponentType::LIGHT_SPOT>::instance()
                          ->registerNewLight("test_spot_light_1", lightTransform);
                spotLight1.addComponent(Component(ComponentType::LIGHT_SPOT, lId));
                spotLight1.addComponent(Component(ComponentType::TRANSFORM, lightTransform));
                auto lightStruct = LightManager<ComponentType::LIGHT_SPOT>::instance()->getLight(
                    lId);
                lightStruct->ambient = glm::vec3(0.251f, 0.251f, 0.012f);
                lightStruct->diffuse = glm::vec3(0.871f, 0.871f, 0.063f);
                lightStruct->specular = glm::vec3(0.6f, 0.6f, 0.055f);
                lightStruct->attenuationConstantTerm = 1.0e-3;
                lightStruct->attenuationLinearTerm = 1.0e-4;
                lightStruct->attenuationQuadraticTerm = 1.0e-4;
                lightStruct->innerCutOff = 0.95f;
                lightStruct->outerCutOff = 0.8f;

                auto transformStruct = TransformManager::instance()->getTransform(lightTransform);
                transformStruct->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
                transformStruct->setPosition(glm::vec3(0.0f, 0.0f, 60.0f));
                transformStruct->setRotation(glm::rotate(transformStruct->rotation(),
                                                         glm::radians(180.0f),
                                                         glm::vec3(0.0f, 1.0f, 0.0f)));

                lightVisualizationShader.addObject(spotLight1);
            }

            {
                // blue
                GameObject &spotLight2 = ObjectManager::instance()->getObject(
                    ObjectManager::instance()->addObject());
                const auto lightTransform = TransformManager::instance()->registerNewTransform(
                    spotLight2);
                const LightSourceIdentifier lId
                    = LightManager<ComponentType::LIGHT_SPOT>::instance()
                          ->registerNewLight("test_spot_light_2", lightTransform);
                spotLight2.addComponent(Component(ComponentType::LIGHT_SPOT, lId));
                spotLight2.addComponent(Component(ComponentType::TRANSFORM, lightTransform));
                auto lightStruct = LightManager<ComponentType::LIGHT_SPOT>::instance()->getLight(
                    lId);
                lightStruct->ambient = glm::vec3(0.016f, 0.012f, 0.188f);
                lightStruct->diffuse = glm::vec3(0.075f, 0.059f, 0.91f);
                lightStruct->specular = glm::vec3(0.102f, 0.098f, 0.329f);
                lightStruct->attenuationConstantTerm = 1.0e-3;
                lightStruct->attenuationLinearTerm = 1.0e-4;
                lightStruct->attenuationQuadraticTerm = 1.0e-4;
                lightStruct->innerCutOff = 0.95f;
                lightStruct->outerCutOff = 0.85f;

                auto transformStruct = TransformManager::instance()->getTransform(lightTransform);
                transformStruct->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
                transformStruct->setPosition(glm::vec3(0.0f, 0.0f, -60.0f));

                lightVisualizationShader.addObject(spotLight2);
            }

            {
                GameObject &texturedLight1 = ObjectManager::instance()->getObject(
                    ObjectManager::instance()->addObject());
                const auto lightTransform = TransformManager::instance()->registerNewTransform(
                    texturedLight1);
                const LightSourceIdentifier lId
                    = LightManager<ComponentType::LIGHT_TEXTURED_SPOT>::instance()
                          ->registerNewLight("bill_texture_spot_1", lightTransform);
                texturedLight1.addComponent(Component(ComponentType::LIGHT_TEXTURED_SPOT, lId));
                texturedLight1.addComponent(Component(ComponentType::TRANSFORM, lightTransform));

                auto lightStruct = LightManager<ComponentType::LIGHT_TEXTURED_SPOT>::instance()
                                       ->getLight(lId);
                lightStruct->ambient = glm::vec3(0.016f, 0.012f, 0.188f);
                lightStruct->specular = glm::vec3(0.102f, 0.098f, 0.329f);
                lightStruct->attenuationConstantTerm = 1.0e-3;
                lightStruct->attenuationLinearTerm = 1.0e-4;
                lightStruct->attenuationQuadraticTerm = 1.0e-4;
                lightStruct->computeIntrinsics(35, 50);

                // const auto texHandle = glGetTextureHandleARB(spotLightTexture);
                // glMakeTextureHandleResidentARB(texHandle);
                // lightStruct->textureIdx = texHandle;

                TextureManager::instance()->allocateTexture(spotLightTexture);
                // lightStruct->textureIdx = TextureManager::instance()->bindTexture(spotLightTexture);
                
                lightStruct->textureIdx = 0xFF00FF00FF00FF00;

                auto transformStruct = TransformManager::instance()->getTransform(lightTransform);
                transformStruct->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
                transformStruct->setPosition(glm::vec3(45.0f, 0.0f, 0.0f));
                transformStruct->setRotation(glm::rotate(transformStruct->rotation(),
                                                         glm::radians(90.0f),
                                                         glm::vec3(0.0f, -1.0f, 0.0f)));

                lightVisualizationShader.addObject(texturedLight1);
            }
        }

        std::vector<GameObjectIdentifier> movingObjects;
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
            movingObjects.emplace_back(standardObject);

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

            LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()->setLightSourceValidator(
                [](DirectionalLight) -> bool { return true; });
            LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()->initializeLightBuffer();
            LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()->bindLightBuffer(1);

            LightManager<ComponentType::LIGHT_POINT>::instance()->setLightSourceValidator(
                [](PointLight) -> bool { return true; });
            LightManager<ComponentType::LIGHT_POINT>::instance()->initializeLightBuffer();
            LightManager<ComponentType::LIGHT_POINT>::instance()->bindLightBuffer(2);

            LightManager<ComponentType::LIGHT_SPOT>::instance()->setLightSourceValidator(
                [](SpotLight) -> bool { return true; });
            LightManager<ComponentType::LIGHT_SPOT>::instance()->initializeLightBuffer();
            LightManager<ComponentType::LIGHT_SPOT>::instance()->bindLightBuffer(3);

            LightManager<ComponentType::LIGHT_TEXTURED_SPOT>::instance()->setLightSourceValidator(
                [](TexturedSpotLight) -> bool { return true; });
            LightManager<ComponentType::LIGHT_TEXTURED_SPOT>::instance()->initializeLightBuffer();
            LightManager<ComponentType::LIGHT_TEXTURED_SPOT>::instance()->bindLightBuffer(4);

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

            const glm::mat4 projection = glm::perspective(glm::radians(camera->zoom()),
                                                          (float)windowWidth / windowHeight, 0.1f,
                                                          1000.0f);
            const glm::mat4 view = camera->getViewMatrix();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            {
                shaderProgramMain.use();

                shaderProgramMain.setMatrix4("view", view);
                shaderProgramMain.setMatrix4("projection", projection);

                shaderProgramMain.setVec3("viewPos", camera->position());
                shaderProgramMain.setInt("testTexture", TextureManager::instance()->bindTexture(spotLightTexture));

                shaderProgramMain.runShader();

                TextureManager::instance()->unbindTexture(spotLightTexture);
            }

            {
                worldPlaneShader.use();

                if (renderOnlyGrid)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                worldPlaneShader.setMatrix4("view", view);
                worldPlaneShader.setMatrix4("projection", projection);

                worldPlaneShader.setFloat("checkerUnitWidth", 5.0f);
                worldPlaneShader.setFloat("checkerUnitHeight", 5.0f);

                worldPlaneShader.runShader();

                if (renderOnlyGrid)
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
                const float lightPosX = std::cos(time * 0.75f);
                const float lightPosZ = std::sin(time * 0.75f);
                const float lightPosY = std::sin(time * 1.25f);
                const glm::vec3 normalizedLightPos = glm::normalize(
                    glm::vec3(lightPosX, lightPosY, lightPosZ));

                {
                    auto transformStruct = TransformManager::instance()->getTransform(
                        pointLight1Transform);
                    transformStruct->setPosition(normalizedLightPos * lightRotationRadius);
                }

                {
                    auto transformStruct = TransformManager::instance()->getTransform(
                        pointLight2Transform);
                    transformStruct->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
                    transformStruct->setPosition(-normalizedLightPos * lightRotationRadius);
                }
            }

            {
                // just adds some fancy rotations for the obects to test instanced buffer updating
                for (size_t m = 0; m < movingObjects.size(); ++m)
                {
                    auto transformStruct = TransformManager::instance()->getTransform(
                        ObjectManager::instance()
                            ->getObject(movingObjects[m])
                            .getIdentifierForComponent(ComponentType::TRANSFORM));

                    transformStruct->setRotation(
                        glm::rotate(transformStruct->rotation(),
                                    (float)(glm::radians(std::cos(m) * 10.0f) * deltaTime),
                                    glm::vec3((float)(m % 2), 0.0f, (float)((m + 1) % 2))));
                }
            }

            {
                const std::unordered_set<GameObjectIdentifier> objectsWithUpdatedTransforms
                    = TransformManager::instance()->flushUpdates();

                LightManager<ComponentType::LIGHT_POINT>::instance()->updateLightSourceTransform(
                    pointLight1Light);
                LightManager<ComponentType::LIGHT_POINT>::instance()->updateLightSourceTransform(
                    pointLight2Light);

                shaderProgramMain.updateInstancedBuffer(objectsWithUpdatedTransforms);

                glfwSwapBuffers(mainWindow.getRawWindow());
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
