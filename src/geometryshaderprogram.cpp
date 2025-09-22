#include "geometryshaderprogram.h"

GeometryShaderProgram::GeometryShaderProgram(const char *vertexPath,
                                             const char *fragmentPath,
                                             const char *geometryPath)
    : ShaderProgram(vertexPath, fragmentPath),
      _geometryPath{geometryPath}
{
}

void GeometryShaderProgram::compileAndAttachNecessaryShaders(uint id)
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

void GeometryShaderProgram::deleteShaders()
{
    glDeleteShader(_geometryShaderId);
    _geometryShaderId = 0;
}
