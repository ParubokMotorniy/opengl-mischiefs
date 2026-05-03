#include "shadermanager.h"
#include "debugshader.h"
#include "shaderprogram.h"

#include <algorithm>
#include <ranges>

ShaderManager::ShaderManager() { registerShader(std::make_unique<DebugShader>(), "debug_shader"); }

ShaderIdentifier ShaderManager::registerShader(std::unique_ptr<ShaderProgram> &&shader,
                                               const std::string &shaderName)
{
    if (const ShaderIdentifier ti = shaderRegistered(shaderName); ti != InvalidIdentifier)
        return ti;

    _shaders.emplace(++_identifiers, NamedShader{ shaderName, std::move(shader) });
    return _identifiers;
}

void ShaderManager::unregisterShader(ShaderIdentifier id)
{
    if (id == _debugShaderId)
        return;
    const auto shaderPtr = _shaders.find(id);
    if (shaderPtr == _shaders.end())
        return;

    _shaders.erase(shaderPtr);
}

ShaderIdentifier ShaderManager::shaderRegistered(const std::string &shaderName) const
{
    const auto shaderPtr = std::ranges::find_if(_shaders, [&shaderName](const auto &pair) {
        return pair.second.componentName == shaderName;
    });
    return shaderPtr == _shaders.end() ? InvalidIdentifier : shaderPtr->first;
}

bool ShaderManager::shaderRegistered(ShaderIdentifier shaderId) const
{
    return _shaders.find(shaderId) != _shaders.end();
}

ShaderProgram *ShaderManager::getShader(ShaderIdentifier shaderId)
{
    auto shaderPtr = _shaders.find(shaderId);
    return shaderPtr == _shaders.end() ? _shaders.find(_debugShaderId)->second.componentData.get()
                                       : shaderPtr->second.componentData.get();
}

ShaderProgram *ShaderManager::getShader(const std::string &shaderName)
{
    const auto shaderPtr = std::ranges::find_if(_shaders, [&shaderName](const auto &pair) {
        return pair.second.componentName == shaderName;
    });
    return shaderPtr == _shaders.end() ? _shaders.find(_debugShaderId)->second.componentData.get()
                                       : shaderPtr->second.componentData.get();
}

void ShaderManager::initializeShader(ShaderIdentifier shaderId)
{
    auto shaderPtr = _shaders.find(shaderId);
    if (shaderPtr == _shaders.end())
    {
        return;
    }
    shaderPtr->second.componentData->initializeShaderProgram();
}

void ShaderManager::cleanUpGracefully() { _shaders.clear(); }
