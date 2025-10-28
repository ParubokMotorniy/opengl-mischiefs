#pragma once

#include "shaderprogram.h"

#include <unordered_map>

class InstancedShader : public ShaderProgram
{
public:
    InstancedShader(const char *vertexPath, const char *fragmentPath);
    ~InstancedShader();

    void
    runTextureMapping();  // this computes the textures used by the objects and puts them into SSBO
    void runInstancing(); // this instances all the necessary data for the shader.
                          // TODO: instanced buffers are currently bound to VAOs of meshes. Find a
                          // workaround.
                          // TODO: now for each transforma change the buffer has to be recomputed ->
                          // ineffiecient
    void runShader() override;

protected:
    void compileAndAttachNecessaryShaders(uint32_t id) override;
    void deleteShaders() override;

protected:
    uint32_t _texturesSSBO;
    std::unordered_map<GameObjectIdentifier, std::array<int, 3>> _objectsTextureMappings;

    std::unordered_map<MeshIdentifier, size_t> _instancedMeshes;
    std::vector<GLuint> _instancedBufferIds;

private:
    const char *_vertexPath = nullptr;
    const char *_fragmentPath = nullptr;

    uint32_t _vertexShaderId = 0;
    uint32_t _fragmentShaderId = 0;
};
