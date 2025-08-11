#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <math.h>

namespace
{
    constexpr size_t windowWidth = 960;
    constexpr size_t windowHeight = 540;

    float vertices[] = {
        0.5f, 0.5f, 0.0f, 0.318f, 0.89f, 0.263f,    // top right
        0.5f, -0.5f, 0.0f, 0.494f, 0.941f, 0.937f,  // bottom right
        0.25f, 0.25f, 0.0f, 0.839f, 0.192f, 0.353f, // concavity right
        -0.5f, -0.5f, 0.0f, 0.733f, 0.678f, 0.988f, // bottom left
        -0.5f, 0.5f, 0.0f, 0.91, 0.216, 0.922,      // top left
        -0.25f, -0.25f, 0.0f, 0.859f, 0.643f, 0.38f // concavity left
    };

    uint indices[] = {
        0, 1, 2,
        4, 5, 3,
        1, 3, 5,
        4, 0, 2};

    const char *vertexShaderSource = "#version 400 core\n"
                                     "layout (location = 0) in vec3 aPos;\n"
                                     "layout (location = 1) in vec3 aColor;\n"
                                     "out vec4 outColor;\n"
                                     "void main()\n"
                                     "{\n"
                                     "gl_Position = vec4(aPos, 1.0);\n"
                                     "outColor = vec4(aColor, 1.f);\n"
                                     "}\0";

    const char *fragmentShaderSourceOrange = "#version 400 core\n"
                                             "out vec4 fragColor;\n"
                                             "in vec4 outColor;\n"
                                             "uniform vec4 extColor;\n"
                                             "void main()\n"
                                             "{fragColor = normalize(outColor*extColor);}\n";

    const char *fragmentShaderSourceGreen = "#version 400 core\n"
                                            "out vec4 fragColor;\n"
                                            "in vec4 outColor;\n"
                                            "uniform vec4 extColor;\n"
                                            "void main()\n"
                                            "{fragColor = normalize(outColor* extColor);}\n";

    char infoLog[512];
}

void compileShader(uint shaderId)
{
    glCompileShader(shaderId);

    int success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderId, sizeof(infoLog), NULL, infoLog);
        std::cerr << "Shader compilation failed. Details: " << infoLog;
    }
}

void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, windowWidth, windowHeight);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    uint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    compileShader(vertexShader);

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    uint fragmentShaderOrange = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderOrange, 1, &fragmentShaderSourceOrange, NULL);
    compileShader(fragmentShaderOrange);

    uint shaderProgramOrange = glCreateProgram();
    glAttachShader(shaderProgramOrange, vertexShader);
    glAttachShader(shaderProgramOrange, fragmentShaderOrange);
    glLinkProgram(shaderProgramOrange);

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    uint fragmentShaderGreen = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderGreen, 1, &fragmentShaderSourceGreen, NULL);
    compileShader(fragmentShaderGreen);

    uint shaderProgramGreen = glCreateProgram();
    glAttachShader(shaderProgramGreen, vertexShader);
    glAttachShader(shaderProgramGreen, fragmentShaderGreen);
    glLinkProgram(shaderProgramGreen);

    glBindVertexArray(0);

    //// Cleanup

    glDeleteShader(fragmentShaderOrange);
    glDeleteShader(fragmentShaderGreen);
    glDeleteShader(vertexShader);

    //// Render loop

    while (!glfwWindowShouldClose(mainWindow))
    {
        processInput(mainWindow);

        glClear(GL_COLOR_BUFFER_BIT);

        const float time = glfwGetTime();
        const float rComponent = std::sin(time / 50) * 2.0f;
        const float gComponent = std::sin(time / 25) * 2.0f;
        const float bComponent = std::sin(time / 5) * 2.0f;

        std::cout << rComponent << gComponent << bComponent << std::endl;

        const int extLocationOrange = glGetUniformLocation(shaderProgramOrange, "extColor");
        glUseProgram(shaderProgramOrange);
        glUniform4f(extLocationOrange, rComponent, gComponent, bComponent, 1.0f);
        glBindVertexArray(VAOOrange);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        const int extLocationGreen = glGetUniformLocation(shaderProgramGreen, "extColor");
        glUseProgram(shaderProgramGreen);
        glUniform4f(extLocationGreen, rComponent, gComponent, bComponent, 1.0f);
        glBindVertexArray(VAOGreen);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(mainWindow);

        glfwPollEvents();
    }

    //// Cleanup

    glDeleteBuffers(1, &VBO);

    glDeleteVertexArrays(1, &VAOOrange);
    glDeleteVertexArrays(1, &VAOGreen);

    glDeleteProgram(shaderProgramOrange);
    glDeleteProgram(shaderProgramGreen);

    glfwTerminate();

    return 0;
}
