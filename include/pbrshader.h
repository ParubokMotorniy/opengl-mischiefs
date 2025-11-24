#pragma once

#include "shaderprogram.h"

#include <string>

class PbrShader : public ShaderProgram
{
public:
    PbrShader(const char *vertexPath, const char *fragmentPath);
    void runShader() override;

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override;
    void deleteShaders() override;

private:
    std::string _vertexPath;
    std::string _fragmentPath;

    uint32_t _vertexShaderId = 0;
    uint32_t _fragmentShaderId = 0;
};
