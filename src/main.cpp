// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
#include "shadowpass.h"
#include "skyboxshader.h"
#include "standardpass.h"
#include "texture.h"
#include "texturemanager.h"
#include "texturemanager3d.h"
#include "timemanager.h"
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

const char *vertexShaderSource = ENGINE_SHADERS "/vertex_standard.vs";
const char *fragmentShaderSource = ENGINE_SHADERS "/fragment_standard.fs";

const char *axesVertexShaderSource = ENGINE_SHADERS "/axis_vertex.vs";
const char *axesFragmentShaderSource = ENGINE_SHADERS "/axis_fragment.fs";
const char *axesGeometryShaderSource = ENGINE_SHADERS "/axis_geometry.gs";

Camera *camera = new QuaternionCamera(glm::vec3(10.f, 10.0f, -10.0f));
const float lightRotationRadius = 50.0f;

bool renderAxes = true;

#ifndef NDEBUG
void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                            const GLchar *message, const void *userParam)
{
    // if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
    //     return;

    std::cerr << "---------------\n";
    std::cerr << "OpenGL Debug Message (" << id << "): " << message << "\n";

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        std::cerr << "Source: API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cerr << "Source: Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cerr << "Source: Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cerr << "Source: Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cerr << "Source: Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        std::cerr << "Source: Other";
        break;
    }
    std::cerr << "\n";

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        std::cerr << "Type: Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cerr << "Type: Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cerr << "Type: Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cerr << "Type: Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cerr << "Type: Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        std::cerr << "Type: Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cerr << "Type: Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cerr << "Type: Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cerr << "Type: Other";
        break;
    }
    std::cerr << "\n";

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cerr << "Severity: high";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cerr << "Severity: medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cerr << "Severity: low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cerr << "Severity: notification";
        break;
    }
    std::cerr << "\n\n";
}
#endif

void imGuiInitialization(Window *windowToBindTo)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; 

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(windowToBindTo->getRawWindow(),
                                 true); 
                                        
    ImGui_ImplOpenGL3_Init();
}

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

    imGuiInitialization(&mainWindow);

#ifndef NDEBUG
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    glViewport(0, 0, windowWidth, windowHeight);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    //// Meshes

    const MeshIdentifier dummyAxesMesh = MeshManager::instance()->registerMesh(Mesh(),
                                                                               "dummy_mesh");

    // Textures

    const TextureIdentifier polyBlack = TextureManager::instance()
                                            ->registerTexture(ENGINE_TEXTURES "/poly_black.jpg",
                                                              "polyBlack");

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

    const TextureIdentifier3D simpleSkybox = TextureManager3D::instance()->registerTexture(
        { ENGINE_TEXTURES "/blue_skybox/right1.png", ENGINE_TEXTURES "/blue_skybox/left2.png",
          ENGINE_TEXTURES "/blue_skybox/top3.png", ENGINE_TEXTURES "/blue_skybox/bottom4.png",
          ENGINE_TEXTURES "/blue_skybox/front5.png", ENGINE_TEXTURES "/blue_skybox/back6.png" },
        "simple_skybox");

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
              ->registerMaterial(BasicMaterial{ polyBlack, specular, floppaEm }, "floppa_material");

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

    const GameObjectIdentifier sphereModel = ModelLoader::instance()->loadModel(
        ENGINE_MODELS "/sphere/sphere.obj");

    const MeshIdentifier sphereMesh = MeshManager::instance()->meshRegistered("Sphere");

    const GameObjectIdentifier cubeModel = ModelLoader::instance()->loadModel(ENGINE_MODELS
                                                                              "/cube/cube.obj");
    const MeshIdentifier cubeMesh = MeshManager::instance()->meshRegistered("Cube");

    const GameObjectIdentifier pyramidModel = ModelLoader::instance()->loadModel(
        ENGINE_MODELS "/pyramid/pyramid.obj");
    const MeshIdentifier pyramidMesh = MeshManager::instance()->meshRegistered("Pyramid");

    {
        //// Shaders

        InstancedShader shaderProgramMain{ vertexShaderSource, fragmentShaderSource };
        shaderProgramMain.initializeShaderProgram();

        WorldPlaneShader worldPlaneShader{ cubeMesh, checkerboardTexture };
        worldPlaneShader.initializeShaderProgram();

        LightVisualizationShader lightVisualizationShader{ sphereMesh };
        lightVisualizationShader.initializeShaderProgram();

        SkyboxShader mainSkybox{ cubeMesh, simpleSkybox };
        mainSkybox.initializeShaderProgram();

        GeometryShaderProgram worldAxesShader{ axesVertexShaderSource, axesFragmentShaderSource,
                                               axesGeometryShaderSource, dummyAxesMesh };
        worldAxesShader.initializeShaderProgram();

        StandardPass _standardRenderingPass{ &shaderProgramMain, &worldPlaneShader,
                                             &lightVisualizationShader, &mainSkybox };
        _standardRenderingPass.setCamera(camera);
        _standardRenderingPass.setWindow(&mainWindow);

        ShadowPass _shadowPass{ &shaderProgramMain, &lightVisualizationShader };

        // Events
        {
            mainWindow.subscribeEventListener([&](KeyboardInput input, KeyboardInput releasedKeys) {
                if (releasedKeys.CtrlLeft)
                    renderAxes = !renderAxes;
                if (releasedKeys.CtrlRight)
                    worldPlaneShader.setPlaneEnabled(!worldPlaneShader.isPlaneEnabled());
            });
            mainWindow.subscribeEventListener(
                [camPtr = camera](KeyboardInput input, KeyboardInput releasedKeys) {
                    camPtr->processKeyboard(input, TimeManager::instance()->getDeltaTime());
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

        // lights

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
            lightStruct->ambient = glm::vec3(0.329f, 0.578f, 0.475f);
            lightStruct->diffuse = glm::vec3(0.622f, 0.535f, 0.198f);
            lightStruct->specular = glm::vec3(0.691f, 0.478f, 0.057f);
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
            lightStruct->ambient = glm::vec3(0.489f, 0.339f, 0.269f);
            lightStruct->diffuse = glm::vec3(0.469f, 0.345f, 0.271f);
            lightStruct->specular = glm::vec3(0.498f, 0.402f, 0.408f);
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
                lightStruct->ambient = glm::vec3(0.029f, 0.078f, 0.075f);
                lightStruct->diffuse = glm::vec3(0.122f, 0.135f, 0.298f);
                lightStruct->specular = glm::vec3(0.091f, 0.078f, 0.057f);

                const auto [bufId, textureIdentifier]
                    = LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()
                          ->createShadowMapPremises(2048, 2048);
                lightStruct->frameBufferId = bufId;
                lightStruct->shadowMapIdentifier = textureIdentifier;

                auto transformStruct = TransformManager::instance()->getTransform(lightTransform);
                transformStruct->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
                transformStruct->setPosition(glm::vec3(0.0f, 30.0f, 0.0f));
                transformStruct->setRotation(glm::rotate(glm::identity<glm::mat4>(),
                                                         glm::radians(90.0f),
                                                         glm::vec3(1.0f, 0.0f, 0.0f)));

                lightStruct->setProjectionMatrix(
                    glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 0.01f, 300.0f));

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
                lightStruct->ambient = glm::vec3(0.089f, 0.039f, 0.069f);
                lightStruct->diffuse = glm::vec3(0.169f, 0.245f, 0.171f);
                lightStruct->specular = glm::vec3(0.098f, 0.002f, 0.008f);
                const auto [bufId, textureIdentifier]
                    = LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()
                          ->createShadowMapPremises(2048, 2048);
                lightStruct->frameBufferId = bufId;
                lightStruct->shadowMapIdentifier = textureIdentifier;

                auto transformStruct = TransformManager::instance()->getTransform(lightTransform);
                transformStruct->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
                transformStruct->setPosition(glm::vec3(0.0f, -30.0f, 0.0f));
                transformStruct->setRotation(glm::rotate(glm::identity<glm::mat4>(),
                                                         glm::radians(90.0f),
                                                         glm::vec3(-1.0f, 0.0f, 0.0f)));

                lightStruct->setProjectionMatrix(
                    glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 0.01f, 300.0f));

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
                lightStruct->diffuse = glm::vec3(0.671f, 0.871f, 0.063f);
                lightStruct->specular = glm::vec3(0.6f, 0.6f, 0.055f);
                lightStruct->attenuationConstantTerm = 1.0e-3;
                lightStruct->attenuationLinearTerm = 1.0e-4;
                lightStruct->attenuationQuadraticTerm = 1.0e-4;
                lightStruct->computeIntrinsics(45, 60);

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
                lightStruct->computeIntrinsics(45, 60);

                auto transformStruct = TransformManager::instance()->getTransform(lightTransform);
                transformStruct->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
                transformStruct->setPosition(glm::vec3(0.0f, 0.0f, -60.0f));

                lightVisualizationShader.addObject(spotLight2);
            }
        }
        GameObject &texturedLight1 = ObjectManager::instance()->getObject(
            ObjectManager::instance()->addObject());
        const auto texturedLight1Transform = TransformManager::instance()->registerNewTransform(
            texturedLight1);
        const LightSourceIdentifier texturedLight1Light
            = LightManager<ComponentType::LIGHT_TEXTURED_SPOT>::instance()
                  ->registerNewLight("bill_texture_spot_1", texturedLight1Transform);
        {
            texturedLight1.addComponent(
                Component(ComponentType::LIGHT_TEXTURED_SPOT, texturedLight1Light));
            texturedLight1.addComponent(
                Component(ComponentType::TRANSFORM, texturedLight1Transform));

            auto lightStruct = LightManager<ComponentType::LIGHT_TEXTURED_SPOT>::instance()
                                   ->getLight(texturedLight1Light);
            lightStruct->ambient = glm::vec3(0.016f, 0.012f, 0.088f);
            lightStruct->specular = glm::vec3(0.102f, 0.098f, 0.129f);
            lightStruct->attenuationConstantTerm = 1.0e-5;
            lightStruct->attenuationLinearTerm = 1.0e-5;
            lightStruct->attenuationQuadraticTerm = 1.0e-6;
            lightStruct->computeIntrinsics(20, 25, texturedLight1Transform);

            TextureManager::instance()->allocateTexture(spotLightTexture);
#if ENGINE_DISABLE_BINDLESS_TEXTURES
            lightStruct->textureIdx = 0xFF00FF00FF00FF00;
#else
            const auto texHandle = glGetTextureHandleARB(
                *TextureManager::instance()->getTexture(spotLightTexture));
            glMakeTextureHandleResidentARB(texHandle);
            lightStruct->textureIdx = texHandle;
#endif

            auto transformStruct = TransformManager::instance()->getTransform(
                texturedLight1Transform);
            transformStruct->setScale(glm::vec3(2.0f, 2.0f, 2.0f));
            transformStruct->setPosition(glm::vec3(45.0f, 0.0f, 0.0f));
            transformStruct->setRotation(glm::rotate(transformStruct->rotation(),
                                                     glm::radians(90.0f),
                                                     glm::vec3(0.0f, -1.0f, 0.0f)));

            lightVisualizationShader.addObject(texturedLight1);
        }

        std::vector<GameObjectIdentifier> movingObjects;
        for (int k = 10; k < 40; ++k)
        {
            const float fK = static_cast<float>(k);
            // add cubes and pyramids
            GameObject &standardObject = ObjectManager::instance()->getObject(
                ObjectManager::instance()->addObject());

            standardObject.addComponent(
                Component(ComponentType::MESH, (k % 20 < 10) ? cubeMesh : pyramidMesh));
            standardObject.addComponent(Component(ComponentType::BASIC_MATERIAL,
                                                  (k % 20 < 10) ? floppaMaterial : catMaterial));

            const TransformIdentifier tId = TransformManager::instance()->registerNewTransform(
                standardObject);
            standardObject.addComponent(Component(ComponentType::TRANSFORM, tId));

            Transform *t = TransformManager::instance()->getTransform(tId);
            t->setPosition(
                glm::vec3(std::sin(k) * 10.0f, (fK / 2) * std::cos(k), (fK / 2) * std::sin(k)));
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
            // auto billCopy = ObjectManager::instance()->copyObject(billModel);
            // auto billTransform = TransformManager::instance()->getTransform(
            //     ObjectManager::instance()->getObject(billCopy).getIdentifierForComponent(
            //         ComponentType::TRANSFORM));
            // billTransform->setPosition(
            //     glm::vec3(-k * std::sin(k), k * std::sin(k), k * std::cos(k)));
            // billTransform->setRotation(
            //     glm::rotate(billTransform->rotation(), glm::radians((float)k),
            //                 glm::vec3(0.0f, (float)(k % 2), (float)((1 + k) % 2))));
            // billTransform->setScale(glm::vec3(20.0f));

            // shaderProgramMain.addObjectWithChildren(billCopy);
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

            camera->moveTo(glm::vec3(20.0f, 7.0f, 0.0f));
            camera->lookAt(glm::vec3(-10.0f, 7.0f, -1.0f));
        }
        //// Render loop
        while (!mainWindow.shouldClose())
        {
            TimeManager::instance()->update();

            const float time = TimeManager::instance()->getTime();
            const float deltaTime = TimeManager::instance()->getDeltaTime();

            mainWindow.update();

            {
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();
                // ImGui::ShowDemoWindow();
            }

            _shadowPass.runPass();
            _standardRenderingPass.runPass();

            // TODO: move this into some gizmos pass
            if (renderAxes)
            {
                glDisable(GL_DEPTH_TEST);
                worldAxesShader.use();

                worldAxesShader.setMatrix4("viewMat", camera->getViewMatrix());
                worldAxesShader.setMatrix4("projectionMat",
                                           glm::perspective(glm::radians(camera->zoom()),
                                                            (float)windowWidth / windowHeight, 0.1f,
                                                            1000.0f));

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
                    transformStruct->setPosition(-normalizedLightPos * lightRotationRadius);
                }

                {
                    // periodically rotates the textured spot light
                    auto transformStruct = TransformManager::instance()->getTransform(
                        texturedLight1Transform);
                    transformStruct->setRotation(
                        glm::rotate(transformStruct->rotation(),
                                    (float)(glm::radians(std::sin(time) * 20.0f) * deltaTime),
                                    glm::vec3(0.0f, 1.0f, 0.0f)));
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
                // Rendering
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }

            {
                const std::unordered_set<GameObjectIdentifier> objectsWithUpdatedTransforms
                    = TransformManager::instance()->flushUpdates();

                LightManager<ComponentType::LIGHT_POINT>::instance()->updateLightSourceTransform(
                    pointLight1Light);
                LightManager<ComponentType::LIGHT_POINT>::instance()->updateLightSourceTransform(
                    pointLight2Light);
                LightManager<ComponentType::LIGHT_TEXTURED_SPOT>::instance()
                    ->updateLightSourceTransform(texturedLight1Light);

                shaderProgramMain.updateInstancedBuffer(objectsWithUpdatedTransforms);

                assert(glGetError() == GL_NO_ERROR);
                glfwSwapBuffers(mainWindow.getRawWindow());
            }
        }
    }
    //// Cleanup
    MeshManager::instance()->cleanUpGracefully();
    TextureManager::instance()->cleanUpGracefully();
    TextureManager3D::instance()->cleanUpGracefully();

    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    glfwTerminate();

    return 0;
}
