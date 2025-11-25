#pragma once

#include "instancer.h"
#include "material.h"
#include "shaderprogram.h"

#include <unordered_map>
#include <unordered_set>

template <typename MaterialStruct>
class InstancedShader : public ShaderProgram
{
public:
    InstancedShader(const char *vertexPath, const char *fragmentPath);
    ~InstancedShader();

    void
    runTextureMapping();  // this computes the textures used by the objects and puts them into SSBO
    void runInstancing(); // this instances all the necessary data for the shader.

    void updateInstancedBuffer(const std::unordered_set<GameObjectIdentifier> &bjsToUpdate);

    void runShader() override;

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override;
    void deleteShaders() override;

protected:
    uint32_t _texturesSSBO = 0;
    std::unordered_map<GameObjectIdentifier,
                       std::array<int, getNumTexturesInMaterial<MaterialStruct>()>>
        _objectsTextureMappings;

    std::unordered_map<MeshIdentifier, size_t> _instancedMeshes;
    std::vector<GLuint> _instancedBufferIds;
    const std::vector<GameObjectIdentifier> &getShaderObjectsAsVector();

    virtual std::vector<InstancedDataGenerator> getDataGenerators();

    size_t _textureHandlesbindingPoint = 0;

private:
    const char *_vertexPath = nullptr;
    const char *_fragmentPath = nullptr;

    uint32_t _vertexShaderId = 0;
    uint32_t _fragmentShaderId = 0;
};

template class InstancedShader<BasicMaterial>;
using InstancedBlinnPhongShader = InstancedShader<BasicMaterial>;

template class InstancedShader<PbrMaterial>;