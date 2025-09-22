#include "meshmanager.h"

#include <glad/glad.h>

#include <cassert>

MeshManager *MeshManager::_instance = nullptr;

MeshManager *MeshManager::instance()
{
    if (_instance == nullptr)
        _instance = new MeshManager();
    return _instance;
}

void MeshManager::registerMesh(const std::string name, const Mesh &&mesh)
{
    _meshes.insert_or_assign(name, mesh);
}

void MeshManager::unregisterMesh(const std::string meshName)
{
    const auto meshPtr = _meshes.find(meshName);
    assert(meshPtr != _meshes.end());
    if (meshPtr == _meshes.end())
        return;

    if (_boundMesh != 0 && meshPtr->second == _boundMesh)
        return;

    _meshes.erase(meshName);
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
