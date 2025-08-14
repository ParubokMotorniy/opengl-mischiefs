#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"

#include <iostream>
#include <cmath>
#include <tuple>

namespace
{
    constexpr size_t windowWidth = 960;
    constexpr size_t windowHeight = 540;

    float vertices[] = {
        //coords                //color                     //shift     //texture
        15.5f, 10.5f, -3.0f,     0.318f, 0.89f, 0.263f,     0.1f,       1.0f, 1.0f,      // top right
        4.5f, -8.5f, 7.0f,       0.494f, 0.941f, 0.937f,    -0.04f,     1.0f, 0.0f,  // bottom right
        -12.25f, 9.25f, 10.0f,   0.839f, 0.192f, 0.353f,    0.3f,       0.5f, 0.5f,   // concavity right
        -16.5f, -7.5f, 5.0f,     0.733f, 0.678f, 0.988f,    -0.5f,      0.0f, 0.0f,  // bottom left
        -6.5f, 7.5f, 3.0f,       0.91, 0.216, 0.922,        0.31f,      0.0f, 1.0f,       // top left
        -10.25f, -18.25f, 3.0f,  0.859f, 0.643f, 0.38f,     -0.15f,     0.5f, 0.5f // concavity left
    };

    uint indices[] = {
        0, 1, 2,
        4, 5, 3,
        1, 3, 5,
        4, 0, 2};

    const char *vertexShaderSource = "./shaders/vertex.vs";

    const char *fragmentShaderSourceOrange = "./shaders/fragment_orange.fs";

    const char *fragmentShaderSourceGreen = "./shaders/fragment_green.fs";

    float oscDirection = 0.0;

    Camera camera;
    float lastX{0.0f};
    float lastY{0.0f};
    float deltaTime{0.0f};
    float previousTime{0.0f};
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

    camera.ProcessKeyboard(MovementInput{
                            glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS, 
                            glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS, 
                            glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS, 
                            glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS}, deltaTime);
}

void mouseCallback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
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
    glEnable(GL_DEPTH_TEST);

    //// Textures

    int width, height, numChannels;
    auto *imageData = stbi_load("./textures/pepe.jpg", &width, &height, &numChannels, 0);

    uint texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(imageData);

    ////////////

    imageData = stbi_load("./textures/nature.jpg", &width, &height, &numChannels, 0);

    uint texture2;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(imageData);

    //// Orange

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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) / 2, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_TRUE, 9 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_TRUE, 9 * sizeof(float), (void *)(7 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    //// Green

    uint VAOGreen;
    glGenVertexArrays(1, &VAOGreen);
    glBindVertexArray(VAOGreen);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    uint EBOGreen;
    glGenBuffers(1, &EBOGreen);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOGreen);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) / 2, indices + 6, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_TRUE, 9 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_TRUE, 9 * sizeof(float), (void *)(7 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    {
        //// Shaders

        Shader shaderProgramOrange{vertexShaderSource, fragmentShaderSourceOrange};
        shaderProgramOrange.use();
        glUniform1i(glGetUniformLocation(shaderProgramOrange, "textureSampler1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgramOrange, "textureSampler2"), 1);

        Shader shaderProgramGreen{vertexShaderSource, fragmentShaderSourceGreen};
        shaderProgramGreen.use();
        glUniform1i(glGetUniformLocation(shaderProgramGreen, "textureSampler1"), 0);
        glUniform1i(glGetUniformLocation(shaderProgramGreen, "textureSampler2"), 1);

        //// Transformation stuff

        glm::mat4 model(1.0f);
        model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        //// Render loop

        camera.LookAt(glm::vec3(10.0f, 10.0f, 10.0f));
        while (!glfwWindowShouldClose(mainWindow))
        {
            deltaTime = glfwGetTime() - previousTime;
            previousTime = glfwGetTime();

            processInput(mainWindow);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            const float time = glfwGetTime();
            const float rComponent = std::sin(time / 50) * 2.0f;
            const float gComponent = std::sin(time / 25) * 2.0f;
            const float bComponent = std::sin(time / 5) * 2.0f;
            const float oscFraction = std::sin(time) * 0.2f;

            const float oscX = std::cos(oscDirection);
            const float oscY = std::sin(oscDirection);

            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / windowHeight, 0.1f, 1000.0f);
            glm::mat4 view = camera.GetViewMatrix();

            for (size_t p = 0; p < 4; ++p)
            {
                const glm::mat4 customModel = glm::translate(model, glm::vec3(p % 2, p, 2 - p));

                for (auto [shaderProgram, vertexArray, oscFractionMultiplier] : {std::tuple<Shader *, uint, float>{&shaderProgramOrange, VAOOrange, 3.0f}, std::tuple<Shader *, uint, float>{&shaderProgramGreen, VAOGreen, 1.5f}})
                {
                    shaderProgram->use();

                    glUniform4f(glGetUniformLocation(*shaderProgram, "extColor"), rComponent, gComponent, bComponent, 1.0f);
                    glUniform3f(glGetUniformLocation(*shaderProgram, "oscillationDirection"), oscX, oscY, 0.0f);
                    glUniform1f(glGetUniformLocation(*shaderProgram, "oscillationFraction"), oscFraction * oscFractionMultiplier);

                    shaderProgram->setMatrix4("model", customModel);
                    shaderProgram->setMatrix4("view", view);
                    shaderProgram->setMatrix4("projection", projection);

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texture1);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, texture2);

                    glBindVertexArray(vertexArray);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                }
            }

            glfwSwapBuffers(mainWindow);

            glfwPollEvents();
        }
    }
    //// Cleanup

    glDeleteBuffers(1, &VBO);

    glDeleteVertexArrays(1, &VAOOrange);
    glDeleteVertexArrays(1, &VAOGreen);

    glfwTerminate();

    return 0;
}
