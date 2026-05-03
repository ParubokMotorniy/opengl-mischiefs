#pragma once

#include "meshmanager.h"
#include "shaderprogram.h"

class DebugShader : public ShaderProgram
{
public:
    void runShader() override
    {
        MeshManager::instance()->bindMesh(MeshManager::instance()->getDummyMesh());
        glDrawArrays(GL_TRIANGLES, 0, 3);
        MeshManager::instance()->unbindMesh();
    }

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override
    {
        if (_vertexShaderId == 0)
        {
            const std::string &vShaderCode = readShaderSource(_vertexPath);

            const char *vPtr = vShaderCode.c_str();

            _vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(_vertexShaderId, 1, &vPtr, 0);
            compileShader(_vertexShaderId);
        }

        glAttachShader(id, _vertexShaderId);

        if (_fragmentShaderId == 0)
        {
            const std::string &fShaderCode = readShaderSource(_fragmentPath);

            const char *fPtr = fShaderCode.c_str();

            _fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(_fragmentShaderId, 1, &fPtr, 0);
            compileShader(_fragmentShaderId);
        }

        glAttachShader(id, _fragmentShaderId);
    }
    void deleteShaders() override
    {
        glDeleteShader(_vertexShaderId);
        _vertexShaderId = 0;

        glDeleteShader(_fragmentShaderId);
        _fragmentShaderId = 0;
    }

private:
    const char *_vertexPath = ENGINE_SHADERS "/debug_magenta.vs";
    const char *_fragmentPath = ENGINE_SHADERS "/debug_magenta.fs";

    uint32_t _vertexShaderId = 0;
    uint32_t _fragmentShaderId = 0;
};