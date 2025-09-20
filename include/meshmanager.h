#pragma once

#include "mesh.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <unordered_set>

class MeshManager
{
public:
    static MeshManager *instance()
    {
        if (_instance == nullptr)
            _instance = new MeshManager();
        return _instance;
    }

    void registerMesh(const std::string name, const Mesh &&mesh)
    {
        _meshes.insert_or_assign(name, mesh);
    }

    void unregisterMesh(const std::string meshName)
    {
        const auto meshPtr = _meshes.find(meshName);
        assert(meshPtr != _meshes.end());
        if (meshPtr == _meshes.end())
            return;

        if (_boundMesh != 0 && meshPtr->second == _boundMesh)
            return;

        _meshes.erase(meshName);
    }

    void allocateMesh(const std::string &meshName)
    {
        const auto meshPtr = _meshes.find(meshName);
        if (meshPtr == _meshes.end())
            return;

        meshPtr->second.allocateMesh();
    }

    void bindMesh(const std::string &meshName)
    {
        if (_boundMesh != 0)
            return;
        const auto meshPtr = _meshes.find(meshName);
        if (meshPtr == _meshes.end())
            return;

        meshPtr->second.bindMesh();
        _boundMesh = meshPtr->second;
    }

    void unbindMesh()
    {
        if (_boundMesh == 0)
            return;
        glBindVertexArray(0);
        _boundMesh = 0;
    }

    void deallocateMesh(const std::string &meshName)
    {
        auto meshPtr = _meshes.find(meshName);
        if (meshPtr == _meshes.end())
            return;

        meshPtr->second.deallocateMesh();
    }

    void cleanUpGracefully()
    {
        unbindMesh();
        _meshes.clear();
    }

    const Mesh *getMesh(const std::string &meshName)
    {
        auto meshPtr = _meshes.find(meshName);
        return meshPtr == _meshes.end() ? nullptr : &meshPtr->second;
    }


private:
    MeshManager() = default;

private:
    // TODO: rework not to use string as identifiers. Convenient but expensive
    static MeshManager *_instance;
    std::unordered_map<std::string, Mesh> _meshes;
    uint _boundMesh = 0;
};
