#pragma once

#include "shaderprogram.h"

#include <unordered_map>

class InstancedShader : public ShaderProgram
{
public:
    InstancedShader(const char *vertexPath, const char *fragmentPath);
    ~InstancedShader();

    void addObject(GameObjectIdentifier gId) override;
    void addObjectWithChildren(GameObjectIdentifier gId) override;
    void runShader() override;

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override;
    void deleteShaders() override;

protected:
    uint32_t _texturesSSBO;
    std::unordered_map<GameObjectIdentifier, std::array<int, 3>> _objectsTextureMappings;

private:
    void runTextureMapping();
    void addObjectWithChildrenImpl(GameObjectIdentifier gId);

private:
    const char *_vertexPath = nullptr;
    const char *_fragmentPath = nullptr;

    uint32_t _vertexShaderId = 0;
    uint32_t _fragmentShaderId = 0;
};
