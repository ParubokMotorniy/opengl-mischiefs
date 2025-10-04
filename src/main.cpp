// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "geometryshaderprogram.h"
#include "instancer.h"
#include "materialmanager.h"
#include "meshmanager.h"
#include "modelloader.h"
#include "object.h"
#include "objectmanager.h"
#include "quaternioncamera.h"
#include "shaderprogram.h"
#include "texture.h"
#include "texturemanager.h"
#include "transformmanager.h"
#include "window.h"

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

const char *lightVertexShaderSource = ENGINE_SHADERS "/light_vertex.vs";
const char *lightFragmentShaderSource = ENGINE_SHADERS "/light_fragment.fs";

const char *axesVertexShaderSource = ENGINE_SHADERS "/axis_vertex.vs";
const char *axesFragmentShaderSource = ENGINE_SHADERS "/axis_fragment.fs";
const char *axesGeometryShaderSource = ENGINE_SHADERS "/axis_geometry.gs";

const char *voronoiDistanceFragmentShaderSource = ENGINE_SHADERS "/voronoi_distances.fs";
const char *voronoiseFragmentShaderSource = ENGINE_SHADERS "/voronoise.fs";

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
    // glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    //// Cubes buffers
    // TODO: make sure the cubes reuse the same mesh but index vertices in their cutsom way
    // TODO: add base plane with checked texture and anisotropic filtering
    const MeshIdentifier simpleCubeMesh = MeshManager::instance()->registerMesh(
        Mesh{ { { -5.0f, -5.0f, -5.0f, 0.0f, 1.0f, -5.0f, -5.0f, -5.0f },
                { -5.0f, -5.0f, 5.0f, 1.0f, 1.0f, -5.0f, -5.0f, 5.0f },
                { -5.0f, 5.0f, 5.0f, 1.0f, 0.0f, -5.0f, 5.0f, 5.0f },
                { -5.0f, 5.0f, -5.0f, 0.0f, 0.0f, -5.0f, 5.0f, -5.0f },

                { 5.0f, -5.0f, -5.0f, 1.0, 1.0f, 5.0f, -5.0f, -5.0f },
                { 5.0f, -5.0f, 5.0f, 0.0f, 1.0f, 5.0f, -5.0f, 5.0f },
                { 5.0f, 5.0f, 5.0f, 0.0f, 0.0f, 5.0f, 5.0f, 5.0f },
                { 5.0f, 5.0f, -5.0f, 1.00f, 0.0f, 5.0f, 5.0f, -5.0f } },

              { 0, 1, 2, 2, 3, 0,

                0, 4, 5, 5, 1, 0,

                4, 5, 6, 6, 7, 4,

                2, 3, 7, 7, 6, 2,

                1, 2, 6, 6, 5, 1,

                0, 3, 7, 7, 4, 0 } },
        "simple_cube");

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

    //   0  4
    // 1 3 5 7
    // 2   6
    const MeshIdentifier simpleCubeMeshUp = MeshManager::instance()->registerMesh(
        Mesh{ { { -5.0f, -5.0f, -5.0f, 0.0f, 1.0f, -5.0f, -5.0f, -5.0f },
                { -5.0f, -5.0f, 5.0f, 1.0f, 1.0f, -5.0f, -5.0f, 5.0f },
                { -5.0f, 5.0f, 5.0f, 1.0f, 0.0f, -5.0f, 5.0f, 5.0f },
                { -5.0f, 5.0f, -5.0f, 0.0f, 0.0f, -5.0f, 5.0f, -5.0f },

                { 5.0f, -5.0f, -5.0f, 1.0, 1.0f, 5.0f, -5.0f, -5.0f },
                { 5.0f, -5.0f, 5.0f, 0.0f, 1.0f, 5.0f, -5.0f, 5.0f },
                { 5.0f, 5.0f, 5.0f, 0.0f, 0.0f, 5.0f, 5.0f, 5.0f },
                { 5.0f, 5.0f, -5.0f, 1.00f, 0.0f, 5.0f, 5.0f, -5.0f } },

              { 3, 0, 1, 2, 1, 5, 6, 5, 4, 7, 4, 0, 4, 5, 1, 3, 2, 6 } },
        "half_cube_up");

    const MeshIdentifier simpleCubeMeshDown = MeshManager::instance()->registerMesh(
        Mesh{ { { -5.0f, -5.0f, -5.0f, 0.0f, 1.0f, -5.0f, -5.0f, -5.0f },
                { -5.0f, -5.0f, 5.0f, 1.0f, 1.0f, -5.0f, -5.0f, 5.0f },
                { -5.0f, 5.0f, 5.0f, 1.0f, 0.0f, -5.0f, 5.0f, 5.0f },
                { -5.0f, 5.0f, -5.0f, 0.0f, 0.0f, -5.0f, 5.0f, -5.0f },

                { 5.0f, -5.0f, -5.0f, 1.0, 1.0f, 5.0f, -5.0f, -5.0f },
                { 5.0f, -5.0f, 5.0f, 0.0f, 1.0f, 5.0f, -5.0f, 5.0f },
                { 5.0f, 5.0f, 5.0f, 0.0f, 0.0f, 5.0f, 5.0f, 5.0f },
                { 5.0f, 5.0f, -5.0f, 1.00f, 0.0f, 5.0f, 5.0f, -5.0f } },

              { 1, 2, 3, 5, 6, 2, 4, 7, 6, 0, 3, 7, 1, 0, 4, 6, 7, 3 } },
        "half_cube_down");

    const MeshIdentifier dummyAxesMesh = MeshManager::instance()->registerMesh(Mesh(),
                                                                               "dummy_mesh");

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

    const MaterialIdentifier floppaMaterial
        = MaterialManager<BasicMaterial, ComponentType::BASIC_MATERIAL>::instance()
              ->registerMaterial(BasicMaterial{ floppaDiff, specular, floppaEm },
                                 "floppa_material");

    const MaterialIdentifier catMaterial
        = MaterialManager<BasicMaterial, ComponentType::BASIC_MATERIAL>::instance()
              ->registerMaterial(BasicMaterial{ catDiff, specular, black }, "cat_material");

    MeshManager::instance()->allocateMesh(simpleCubeMesh);
    MeshManager::instance()->allocateMesh(simplePyramidMesh);
    MeshManager::instance()->allocateMesh(simpleCubeMeshUp);
    MeshManager::instance()->allocateMesh(simpleCubeMeshDown);
    MeshManager::instance()->allocateMesh(dummyAxesMesh);

    TextureManager::instance()->allocateTexture(floppaDiff);
    TextureManager::instance()->allocateTexture(floppaEm);
    TextureManager::instance()->allocateTexture(specular);
    TextureManager::instance()->allocateTexture(black);
    TextureManager::instance()->allocateTexture(catDiff);

    //"Tank - WW1" (https://skfb.ly/oqRNY) by Andy Woodhead is licensed under Creative Commons
    // Attribution (http://creativecommons.org/licenses/by/4.0/).
    // GameObjectIdentifier tankModel = ModelLoader::instance()->loadModel(ENGINE_MODELS
    // "/tank/tank.obj");

    // GameObject &upperCube =
    // ObjectManager::instance()->getObject(ObjectManager::instance()->addObject());
    // {
    //     upperCube.addComponent(Component(ComponentType::MESH, simpleCubeMeshUp));
    //     upperCube.addComponent(Component(ComponentType::TRANSFORM,
    //     TransformManager::instance()->registerNewTransform()));
    // }
    // std::vector<GameObject *> voronoiDistancesObjects = {&upperCube};

    // GameObject &lowerCube =
    // ObjectManager::instance()->getObject(ObjectManager::instance()->addObject());
    // {
    //     lowerCube.addComponent(Component(ComponentType::MESH, simpleCubeMeshDown));
    //     lowerCube.addComponent(Component(ComponentType::TRANSFORM,
    //     TransformManager::instance()->registerNewTransform()));
    // }
    // std::vector<GameObject *> voronoiseObjects = {&lowerCube};

    // std::vector<Model *> standardShaderModels = {&tankModel};
    // std::vector<GameObject> cubeLightObjects = {{.objMesh = "simple_cube", .scale =
    // glm::vec3(0.5f, 0.5f, 0.5f)}}; std::vector<GameObject *> objectsWithAxes =
    // {&standardShaderObjects[0], &voronoiseObjects[0], &voronoiDistancesObjects[0]};

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

    // TODO: make sure the meshes are organized by shaders and rendered in batches, in instanced
    // fashion
    {
        //// Shaders

        ShaderProgram shaderProgramMain{ vertexShaderSource, fragmentShaderSource };
        shaderProgramMain.initializeShaderProgram();

        // ShaderProgram lightCubeShader{lightVertexShaderSource, lightFragmentShaderSource};
        // lightCubeShader.initializeShaderProgram();

        // ShaderProgram voronoiseShader{vertexShaderSource, voronoiseFragmentShaderSource};
        // voronoiseShader.initializeShaderProgram();

        // ShaderProgram voronoiDistancesShader{vertexShaderSource,
        // voronoiDistanceFragmentShaderSource}; voronoiDistancesShader.initializeShaderProgram();

        GeometryShaderProgram worldAxesShader{ axesVertexShaderSource, axesFragmentShaderSource,
                                               axesGeometryShaderSource };
        worldAxesShader.initializeShaderProgram();

        for (int k = 10; k < 160; ++k)
        {
            ///

            GameObject &standardObject = ObjectManager::instance()->getObject(
                ObjectManager::instance()->addObject());

            standardObject.addComponent(
                Component(ComponentType::MESH, (k % 20 < 10) ? simpleCubeMesh : simplePyramidMesh));
            standardObject.addComponent(Component(ComponentType::BASIC_MATERIAL,
                                                  (k % 20 < 10) ? floppaMaterial : catMaterial));

            const TransformIdentifier tId = TransformManager::instance()->registerNewTransform();
            standardObject.addComponent(Component(ComponentType::TRANSFORM, tId));

            Transform *t = TransformManager::instance()->getTransform(tId);
            t->setPosition(glm::vec3(k * std::sin(k), k * std::cos(k), -k * std::sin(k)));
            t->setRotation(glm::rotate(t->rotation(), glm::radians((float)k),
                                       glm::vec3(50.0f - k, 1.0f, 0.0f)));
            t->setScale(glm::vec3(std::max((k % 20) / 10.0f, 0.1f)));

            shaderProgramMain.addObject(standardObject);

            ///

            GameObject &standardAxes = ObjectManager::instance()->getObject(
                ObjectManager::instance()->addObject());
            standardAxes.addComponent(Component(ComponentType::MESH, dummyAxesMesh));
            standardAxes.addComponent(Component(ComponentType::TRANSFORM, tId));

            worldAxesShader.addObject(standardAxes);
        }

        GameObject &worldAxes = ObjectManager::instance()->getObject(
            ObjectManager::instance()->addObject());
        worldAxes.addComponent(Component(ComponentType::MESH, dummyAxesMesh));
        worldAxes.addComponent(Component(ComponentType::TRANSFORM,
                                         TransformManager::instance()->registerNewTransform()));
        worldAxesShader.addObject(worldAxes);

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
            const glm::vec3 normalizedLightPos = glm::normalize(
                glm::vec3(lightPosX, lightPosY, lightPosZ));

            const glm::mat4 projection = glm::perspective(glm::radians(camera->zoom()),
                                                          (float)windowWidth / windowHeight, 0.1f,
                                                          1000.0f);
            const glm::mat4 view = camera->getViewMatrix();

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

            // {
            //     voronoiseShader.use();
            //     voronoiseShader.setVec3("iResolution", glm::vec3(windowWidth, windowHeight,
            //     0.0f)); voronoiseShader.setFloat("iTime", time);
            //     voronoiseShader.setMatrix4("view", view);
            //     voronoiseShader.setMatrix4("projection", projection);

            //     for (const GameObject *obj : voronoiseObjects)
            //     {
            //         const MeshIdentifier meshId =
            //         obj->getIdentifierForComponent(ComponentType::MESH);
            //         MeshManager::instance()->bindMesh(meshId);

            //         voronoiseShader.setMatrix4("model",
            //         TransformManager::instance()->getModelMatrix(obj->getIdentifierForComponent(ComponentType::TRANSFORM)));

            //         glDrawElements(GL_TRIANGLES,
            //         MeshManager::instance()->getMesh(meshId)->numIndices(), GL_UNSIGNED_INT, 0);
            //         MeshManager::instance()->unbindMesh();
            //     }
            // }

            // {
            //     voronoiDistancesShader.use();
            //     voronoiDistancesShader.setVec3("iResolution", glm::vec3(1.0f, 1.0f, 0.0f));
            //     voronoiDistancesShader.setFloat("iTime", time);
            //     voronoiDistancesShader.setMatrix4("view", view);
            //     voronoiDistancesShader.setMatrix4("projection", projection);

            //     for (const GameObject *obj : voronoiDistancesObjects)
            //     {
            //         const MeshIdentifier meshId =
            //         obj->getIdentifierForComponent(ComponentType::MESH);
            //         MeshManager::instance()->bindMesh(meshId);

            //         voronoiDistancesShader.setMatrix4("model",
            //         TransformManager::instance()->getModelMatrix(obj->getIdentifierForComponent(ComponentType::TRANSFORM)));

            //         glDrawElements(GL_TRIANGLES,
            //         MeshManager::instance()->getMesh(meshId)->numIndices(), GL_UNSIGNED_INT, 0);
            //         MeshManager::instance()->unbindMesh();
            //     }

            //     TextureManager::instance()->unbindAllTextures();
            // }

            // {
            //     lightCubeShader.use();

            //     for (const GameObject &lightCube : cubeLightObjects)
            //     {
            //         MeshManager::instance()->bindMesh(lightCube.objMesh);
            //         glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), glm::vec3(lightPosX,
            //         lightPosY, lightPosZ)); lightModel = glm::scale(lightModel, glm::vec3(0.6f,
            //         0.6f, 0.6f));

            //         lightCubeShader.setVec3("actualLightColor", normalizedLightPos);
            //         lightCubeShader.setMatrix4("model", lightModel);
            //         lightCubeShader.setMatrix4("view", view);
            //         lightCubeShader.setMatrix4("projection", projection);

            //         glDrawElements(GL_TRIANGLES,
            //         MeshManager::instance()->getMesh(lightCube.objMesh)->numIndices(),
            //         GL_UNSIGNED_INT, 0); MeshManager::instance()->unbindMesh();
            //     }
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
