#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

#include "meshmanager.h"
#include "objectmanager.h"

// TODO: debloat the shader program
class ShaderProgram
{
public:
    ShaderProgram(const char *vertexPath, const char *fragmentPath);

    void initializeShaderProgram();

    ~ShaderProgram();

    void use() const;

    void setBool(const std::string &name, bool value) const;

    void setInt(const std::string &name, int value) const;

    void setFloat(const std::string &name, float value) const;

    void setMatrix4(const std::string &name, const glm::mat4 &mat);

    void setVec3(const std::string &name, const glm::vec3 &vec);

    void setVec4(const std::string &name, const glm::vec4 &vec);

    void addObject(GameObjectIdentifier gId);
    void addObjectWithChildren(GameObjectIdentifier gId);

    virtual void runShader();

    operator int() { return _id; }

protected:
    virtual void compileAndAttachNecessaryShaders(uint32_t id);
    virtual void deleteShaders();

    void compileShader(uint32_t shaderId);
    void linkProgram(uint32_t programId);

    std::string readShaderSource(const char *shaderSource);
    void runTextureMapping();
    void addObjectWithChildrenImpl(GameObjectIdentifier gId);

protected:
    char _infoLog[512];

    struct MeshOrderer
    {
        bool operator()(const GameObjectIdentifier &gId1, const GameObjectIdentifier &gId2) const
        {
            const MeshIdentifier mId1 = ObjectManager::instance()
                                            ->getObject(gId1)
                                            .getIdentifierForComponent(ComponentType::MESH);
            const MeshIdentifier mId2 = ObjectManager::instance()
                                            ->getObject(gId2)
                                            .getIdentifierForComponent(ComponentType::MESH);
            return mId1 < mId2;
        }
    };
    std::multiset<GameObjectIdentifier, MeshOrderer> _orderedShaderObjects;

    uint32_t _texturesSSBO;
    std::unordered_map<GameObjectIdentifier, std::array<int, 3>> _objectsTextureMappings;

private:
    unsigned int _id;
    const char *_vertexPath = nullptr;
    const char *_fragmentPath = nullptr;
};
