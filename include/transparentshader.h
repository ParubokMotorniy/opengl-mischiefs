#pragma once

#include "glm/glm.hpp"

#include "types.h"
#include "camera.h"
#include "shaderprogram.h"

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
    glm::vec3 _previousCameraPosition;
    float _minimalPreviousDistance = 0.0f;

    std::vector<GameObjectIdentifier> _sortedObjects;

    const char *_vertexPath = nullptr;
    const char *_fragmentPath = nullptr;

    uint32_t _vertexShaderId = 0;
    uint32_t _fragmentShaderId = 0;
};
