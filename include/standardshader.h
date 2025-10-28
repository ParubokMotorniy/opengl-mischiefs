#pragma once

#include "shaderprogram.h"

class StandardShader : public ShaderProgram
{
public:
    StandardShader(const char *vertexPath, const char *fragmentPath);

protected:
    virtual void compileAndAttachNecessaryShaders(uint32_t id) override;
    virtual void deleteShaders() override;
    virtual void updateUniforms() override;
    virtual void makeDrawCalls() override;
};
