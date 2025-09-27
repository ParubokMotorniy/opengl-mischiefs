#include "meshmanager.h"

#include "randomnamer.h"

#include <glad/glad.h>

#include <cassert>

void MeshManager::registerMesh(const Mesh &&mesh, const std::string &name)
{
    _meshes.insert_or_assign(name, mesh);
}

void MeshManager::unregisterMesh(const std::string &meshName)
{
    const auto meshPtr = _meshes.find(meshName);
    assert(meshPtr != _meshes.end());
    if (meshPtr == _meshes.end())
        return;

    if (_boundMesh != 0 && meshPtr->second == _boundMesh)
        return;

    _meshes.erase(meshName);
}

std::string MeshManager::registerMesh(const Mesh &&mesh)
{
    const auto rName = RandomNamer::instance()->getRandomName(10);
    registerMesh(std::move(mesh), rName);
    return rName;
}

bool MeshManager::meshRegistered(const MeshIdentifier &mId)
{
   return _meshes.contains(mId);
}

void MeshManager::allocateMesh(const std::string &meshName)
{
    const auto meshPtr = _meshes.find(meshName);
    if (meshPtr == _meshes.end())
        return;

    meshPtr->second.allocateMesh();
}

void MeshManager::bindMesh(const std::string &meshName)
{
    if (_boundMesh != 0)
        return;
    const auto meshPtr = _meshes.find(meshName);
    if (meshPtr == _meshes.end())
        return;

    meshPtr->second.bindMesh();
    _boundMesh = meshPtr->second;
}

void MeshManager::unbindMesh()
{
    if (_boundMesh == 0)
        return;
    glBindVertexArray(0);
    _boundMesh = 0;
}

void MeshManager::deallocateMesh(const std::string &meshName)
{
    auto meshPtr = _meshes.find(meshName);
    if (meshPtr == _meshes.end())
        return;

    meshPtr->second.deallocateMesh();
}

void MeshManager::cleanUpGracefully()
{
    unbindMesh();
    _meshes.clear();
}

const Mesh *MeshManager::getMesh(const std::string &meshName)
{
    auto meshPtr = _meshes.find(meshName);
    return meshPtr == _meshes.end() ? nullptr : &meshPtr->second;
}
