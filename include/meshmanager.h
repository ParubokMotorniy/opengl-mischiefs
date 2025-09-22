#pragma once

#include "mesh.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <unordered_set>

class MeshManager
{
public:
    static MeshManager *instance();

    void registerMesh(const std::string name, const Mesh &&mesh);

    void unregisterMesh(const std::string meshName);

    void allocateMesh(const std::string &meshName);

    void bindMesh(const std::string &meshName);

    void unbindMesh();

    void deallocateMesh(const std::string &meshName);

    void cleanUpGracefully();

    const Mesh *getMesh(const std::string &meshName);

private:
    MeshManager() = default;

private:
    // TODO: rework not to use string as identifiers. Convenient but expensive
    static MeshManager *_instance;
    std::unordered_map<std::string, Mesh> _meshes;
    uint32_t _boundMesh = 0;
};
