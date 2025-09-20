#pragma once

#include "shaderprogram.h"

class GeometryShaderProgram : public ShaderProgram
{
public:
    GeometryShaderProgram(const char *vertexPath, const char *fragmentPath, const char *geometryPath = nullptr) : ShaderProgram(vertexPath, fragmentPath), _geometryPath{geometryPath} {}

protected:
    void compileAndAttachNecessaryShaders(uint id) override
    {
        if (id == 0)
            return;

        if (_geometryShaderId == 0)
        {
            const std::string &gShaderCode = readShaderSource(_geometryPath);

            const char *gPtr = gShaderCode.c_str();

            _geometryShaderId = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(_geometryShaderId, 1, &gPtr, NULL);
            compileShader(_geometryShaderId);
        }

        glAttachShader(id, _geometryShaderId);
    }

    void deleteShaders() override
    {
        glDeleteShader(_geometryShaderId);
        _geometryShaderId = 0;
    }

private:
    const char *_geometryPath = nullptr;
    uint _geometryShaderId = 0;
};
