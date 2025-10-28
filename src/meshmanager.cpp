#include "meshmanager.h"

#include "randomnamer.h"

#include <glad/glad.h>

#include <algorithm>
#include <cassert>

TextureIdentifier MeshManager::registerMesh(const Mesh &&mesh, const std::string &name)
{
    if (const TextureIdentifier ti = meshRegistered(name); ti != InvalidIdentifier)
        return ti;

    _meshes.emplace(++_identifiers, NamedMesh{ name, mesh });
    return _identifiers;
}

void MeshManager::unregisterMesh(MeshIdentifier id)
{
    const auto meshPtr = _meshes.find(id);
    assert(meshPtr != _meshes.end());
    if (meshPtr == _meshes.end())
        return;

    if (_boundMesh != 0 && id == _boundMesh)
        return;

    _meshes.erase(id);
}

std::pair<std::string, MeshIdentifier> MeshManager::registerMesh(const Mesh &&mesh)
{
    const auto rName = RandomNamer::instance()->getRandomName(10);
    const auto id = registerMesh(std::move(mesh), rName);
    return std::make_pair(rName, id);
}

MeshIdentifier MeshManager::meshRegistered(const std::string &meshName)
{
    const auto meshPtr = std::ranges::find_if(_meshes, [&meshName](const auto &pair) {
        return pair.second.componentName == meshName;
    });
    return meshPtr == _meshes.end() ? InvalidIdentifier : meshPtr->first;
}

void MeshManager::allocateMesh(MeshIdentifier id)
{
    const auto meshPtr = _meshes.find(id);
    if (meshPtr == _meshes.end())
        return;

    meshPtr->second.componentData.allocateMesh();
}

void MeshManager::bindMesh(MeshIdentifier id)
{
    if (_boundMesh != InvalidIdentifier)
        return;
    const auto meshPtr = _meshes.find(id);
    if (meshPtr == _meshes.end())
        return;
    auto &mesh = meshPtr->second.componentData;

    mesh.bindMesh();
    _boundMesh = id;
}

void MeshManager::unbindMesh()
{
    glBindVertexArray(0);
    _boundMesh = InvalidIdentifier;
}

void MeshManager::bindMeshInstanced(MeshIdentifier id)
{
    if(_boundMesh != InvalidIdentifier)
        return;

    auto meshPtr = _meshes.find(id);
    if (meshPtr == _meshes.end())
        return;
    
    auto &mesh = meshPtr->second.componentData; 
    if(!mesh.instancingEnabled())
        return;

    mesh.bindMeshInstanced();
    _boundMesh = id;
}

void MeshManager::enableMeshInstancing(MeshIdentifier id)
{
    auto meshPtr = _meshes.find(id);
    if (meshPtr == _meshes.end())
        return;

    meshPtr->second.componentData.enableInstancing();
}

void MeshManager::deallocateMesh(MeshIdentifier id)
{
    auto meshPtr = _meshes.find(id);
    if (meshPtr == _meshes.end())
        return;

    meshPtr->second.componentData.deallocateMesh();
}

void MeshManager::cleanUpGracefully()
{
    unbindMesh();
    _meshes.clear();
}

const Mesh *MeshManager::getMesh(MeshIdentifier id)
{
    auto meshPtr = _meshes.find(id);
    return meshPtr == _meshes.end() ? nullptr : &meshPtr->second.componentData;
}
