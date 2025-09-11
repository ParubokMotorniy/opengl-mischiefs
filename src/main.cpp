#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "texture.h"

#include <iostream>
#include <cmath>
#include <tuple>

namespace
{
    constexpr size_t windowWidth = 1440;
    constexpr size_t windowHeight = 810;

    // TODO: fix winding order
    //clang-format off
    float vertices[] = {
        // coords                  //color                    //shift     //texture
        -5.0f, -5.0f, -5.0f, 0.318f, 0.89f, 0.263f, 0.1f, 0.0f, 1.0f,   // top right                0   4
        -5.0f, -5.0f, 5.0f, 0.494f, 0.941f, 0.937f, -0.04f, 1.0f, 1.0f, // bottom right             1 3 5 7
        -5.0f, 5.0f, 5.0f, 0.839f, 0.192f, 0.353f, 0.3f, 1.0f, 0.0f,    // concavity right       2   6
        -5.0f, 5.0f, -5.0f, 0.733f, 0.678f, 0.988f, -0.5f, 0.0f, 0.0f,  // bottom left

        5.0f, -5.0f, -5.0f, 0.91, 0.216, 0.922, 0.31f, 1.0f, 1.0f,    // top left
        5.0f, -5.0f, 5.0f, 0.859f, 0.643f, 0.38f, -0.15f, 1.0f, 0.0f, // concavity left
        5.0f, 5.0f, 5.0f, 0.91, 0.216, 0.922, 0.014f, 0.0f, 0.0f,     // top left
        5.0f, 5.0f, -5.0f, 0.859f, 0.643f, 0.38f, -0.65f, 0.0f, 1.0f  // concavity left
    };
    //clang-format on

    uint indices[] = {
        0, 1, 2,
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
        7, 4, 0};

    const char *vertexShaderSource = "./shaders/vertex.vs";
    const char *fragmentShaderSource = "./shaders/fragment.fs";

    const char *lightVertexShaderSource = "./shaders/light_vertex.vs";
    const char *lightFragmentShaderSource = "./shaders/light_fragment.fs";

    const char *axesVertexShaderSource = "./shaders/axis_vertex.vs";
    const char *axesFragmentShaderSource = "./shaders/axis_fragment.fs";
    const char *axesGeometryShaderSource = "./shaders/axis_geometry.gs";

    float oscDirection = 0.0;

    Camera camera;
    float lastX{0.0f};
    float lastY{0.0f};
    float deltaTime{0.0f};
    float previousTime{0.0f};

    const glm::vec3 ambColor = glm::vec3(0.522f, 0.922f, 0.506f);
    const glm::vec3 lightColor = glm::vec3(0.369f, 0.722f, 0.941f);
    const glm::vec3 cubeDiffColor = glm::vec3(0.765f, 0.929f, 0.38f);
    const glm::vec3 cubeSpecColor = glm::vec3(0.91f, 0.839f, 0.729f);
    const float lightRotationRadius = 25.0f;
}

void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        oscDirection += 0.1;

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        oscDirection -= 0.1;

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

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        camera.processMouseMovement(xoffset, yoffset);
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.processMouseScroll(static_cast<float>(yoffset));
}

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

    uint VAOOrange;
    glGenVertexArrays(1, &VAOOrange);
    glBindVertexArray(VAOOrange);

    uint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    uint EBOOrange;
    glGenBuffers(1, &EBOOrange);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOOrange);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_TRUE, 9 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_TRUE, 9 * sizeof(float), (void *)(7 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    //// Light source buffers

    uint VAOLight;
    glGenVertexArrays(1, &VAOLight);
    glBindVertexArray(VAOLight);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    uint EBOLight;
    glGenBuffers(1, &EBOLight);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOLight);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    //// Dummy axes VAO
    uint dummyVao;
    glGenVertexArrays(1, &dummyVao);
    glBindVertexArray(0);

    {
        //// Textures

        Texture2D diffuseTex{"./textures/floppa.jpg"};
        Texture2D specularTex{"./textures/specular.jpg"};
        Texture2D emissionTex{"./textures/floppa_emission.jpg"};

        //// Shaders

        Shader shaderProgramOrange{vertexShaderSource, fragmentShaderSource};
        shaderProgramOrange.use();
        glUniform1i(glGetUniformLocation(shaderProgramOrange, "currentMaterial.diffTextureSampler"), 0);
        glUniform1i(glGetUniformLocation(shaderProgramOrange, "currentMaterial.specTextureSampler"), 1);
        glUniform1i(glGetUniformLocation(shaderProgramOrange, "currentMaterial.emissionTextureSampler"), 2);

        Shader lightCubeShader{lightVertexShaderSource, lightFragmentShaderSource};
        Shader worldAxesShader{axesVertexShaderSource, axesFragmentShaderSource, axesGeometryShaderSource};

        //// Transformation stuff

        glm::mat4 model(1.0f);
        model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        //// Render loop
        camera.lookAt(glm::vec3(10.0f, 10.0f, 10.0f));
        while (!glfwWindowShouldClose(mainWindow))
        {
            deltaTime = glfwGetTime() - previousTime;
            previousTime = glfwGetTime();

            processInput(mainWindow);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            const float time = glfwGetTime();
            const float oscFraction = std::sin(time) * 0.7f;

            const float oscX = std::cos(oscDirection);
            const float oscY = std::sin(oscDirection);

            const float lightPosX = std::cos(time * 2.0f) * lightRotationRadius;
            const float lightPosY = std::sin(time * 2.0f) * lightRotationRadius;
            const float lightPosZ = std::sin(time * 3.0f);
            const glm::vec3 normalizedLightPos = glm::normalize(glm::vec3(lightPosX, lightPosY, lightPosZ));

            const glm::mat4 projection = glm::perspective(glm::radians(camera.zoom()), (float)windowWidth / windowHeight, 0.1f, 1000.0f);
            const glm::mat4 view = camera.getViewMatrix();

            {
                glBindVertexArray(dummyVao);

                worldAxesShader.use();

                worldAxesShader.setMatrix4("viewMat", view);
                worldAxesShader.setMatrix4("clipMat", projection);
                // TODO: add proper rescaling of axes due to zoom
                worldAxesShader.setFloat("axisLength", 0.03l);
                worldAxesShader.setFloat("thickness", 0.0005l);
                worldAxesShader.setFloat("cameraDistance", glm::length(camera.position()));
                worldAxesShader.setFloat("cameraNear", 0.1f);

                glDrawArrays(GL_POINTS, 0, 3);

                glBindVertexArray(0);
            }

            for (size_t p = 0; p < 3; ++p)
            {
                const glm::mat4 customModel = glm::translate(model, glm::vec3(p * 10, p * 10, std::sin(p)));

                shaderProgramOrange.use();

                shaderProgramOrange.setVec3("oscillationDirection", glm::vec3(oscX, oscY, 0.0f));
                shaderProgramOrange.setFloat("oscillationFraction", oscFraction * 3.0f);

                shaderProgramOrange.setVec3("currentMaterial.ambColor", ambColor);
                shaderProgramOrange.setVec3("currentMaterial.diffColor", cubeDiffColor);
                shaderProgramOrange.setVec3("currentMaterial.specColor", cubeSpecColor);

                shaderProgramOrange.setVec3("currentLight.lightPos", {lightPosX, lightPosY, lightPosZ});
                shaderProgramOrange.setVec3("currentLight.diffStrength", normalizedLightPos);
                shaderProgramOrange.setVec3("currentLight.specStrength", {0.8f, 0.8f, 0.8f});
                shaderProgramOrange.setVec3("currentLight.ambStrength", {0.15f, 0.15f, 0.15f});
                shaderProgramOrange.setFloat("currentLight.k", 1.2f);
                shaderProgramOrange.setFloat("currentLight.b", 0.1f);

                shaderProgramOrange.setVec3("viewPos", camera.position());

                shaderProgramOrange.setMatrix4("model", customModel);
                shaderProgramOrange.setMatrix4("view", view);
                shaderProgramOrange.setMatrix4("projection", projection);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, diffuseTex);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, specularTex);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, emissionTex);

                glBindVertexArray(VAOOrange);
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

                glBindVertexArray(0);
            }

            {
                lightCubeShader.use();
                glm::mat4 lightModel = glm::translate(glm::mat4(1.0f), glm::vec3(lightPosX, lightPosY, lightPosZ));
                lightModel = glm::scale(lightModel, glm::vec3(0.6f, 0.6f, 0.6f));

                lightCubeShader.setVec3("actualLightColor", normalizedLightPos);
                lightCubeShader.setMatrix4("model", lightModel);
                lightCubeShader.setMatrix4("view", view);
                lightCubeShader.setMatrix4("projection", projection);

                glBindVertexArray(VAOLight);
                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

                glBindVertexArray(0);
            }

            glfwSwapBuffers(mainWindow);

            glfwPollEvents();
        }
    }
    //// Cleanup

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAOOrange);

    glfwTerminate();

    return 0;
}
