#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

#include "meshmanager.h"
#include "objectmanager.h"

class ShaderProgram
{
public:
    ShaderProgram();
    virtual ~ShaderProgram();

    void initializeShaderProgram();

    void use() const;
    GLuint programId() const;

    void setShaderProgramOverride(GLuint programId);
    void removeShaderProgramOverride();

    void setBool(const std::string &name, bool value) const;

    void setInt(const std::string &name, int value) const;

    void setFloat(const std::string &name, float value) const;

    void setMatrix4(const std::string &name, const glm::mat4 &mat) const;

    void setVec3(const std::string &name, const glm::vec3 &vec) const;

    void setVec4(const std::string &name, const glm::vec4 &vec) const;

    void setUvec2(const std::string &name, GLuint64 value) const;

    virtual void addObject(GameObjectIdentifier gId);
    virtual void addObjectWithChildren(GameObjectIdentifier gId);

    virtual void runShader() = 0;

    operator int() const { return _id; }

protected:
    virtual void compileAndAttachNecessaryShaders(uint32_t id) = 0;
    virtual void deleteShaders() = 0;

    void compileShader(uint32_t shaderId);
    void linkProgram(uint32_t programId);

    std::string readShaderSource(const char *shaderSource);

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
    // the objects of the shader are ordered by meshes to enable easier batching of meshes
    std::multiset<GameObjectIdentifier, MeshOrderer> _orderedShaderObjects;

private:
    unsigned int _id = 0;
    std::optional<GLuint> _shaderOverride = std::nullopt;
};
