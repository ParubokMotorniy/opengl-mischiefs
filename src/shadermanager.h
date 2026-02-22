#pragma once

#include "singleton.h"
#include "types.h"

#include <memory>
#include <unordered_map>

class ShaderProgram;

class ShaderManager : public SystemSingleton<ShaderManager>
{
    friend class SystemSingleton; // so that the singleton can access the private constructor
    using NamedShader = NamedComponent<std::unique_ptr<ShaderProgram>>;

public:
    ShaderIdentifier registerShader(std::unique_ptr<ShaderProgram> &&shader,
                                    const std::string &shaderName);
    void unregisterShader(ShaderIdentifier id);

    ShaderIdentifier shaderRegistered(const std::string &shaderName) const;
    bool shaderRegistered(ShaderIdentifier shaderId) const;

    ShaderProgram *getShader(ShaderIdentifier tId);
    ShaderProgram *getShader(const std::string &shaderName);
    void initializeShader(ShaderIdentifier id);

    void cleanUpGracefully();

private:
    ShaderManager();

private:
    ShaderIdentifier _identifiers = 0;
    ShaderIdentifier _debugShaderId = 1;
    std::unordered_map<ShaderIdentifier, NamedShader> _shaders;
};