#pragma once

#include "glm/glm.hpp"

#include "camera.h"
#include "shaderprogram.h"
#include "types.h"

#include <vector>

class TransparentShader : public ShaderProgram
{
public:
    TransparentShader(const char *vertexPath, const char *fragmentPath);
    void runShader() override;
    void setCamera(const Camera *newCamera);

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override;
    void deleteShaders() override;

private:
    const Camera *_currentCamera = nullptr;
    glm::vec3 _previousCameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    float _minimalPreviousDistance = 0.0f;

    std::vector<GameObjectIdentifier> _sortedObjects;

    std::string _vertexPath;
    std::string _fragmentPath;

    uint32_t _vertexShaderId = 0;
    uint32_t _fragmentShaderId = 0;
};
