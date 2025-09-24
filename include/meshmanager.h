#pragma once

#include "mesh.h"
#include "types.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <unordered_set>

class MeshManager
{
public:
    static MeshManager *instance();

    MeshManager(const MeshManager &other) = delete;
    MeshManager(MeshManager &&other) = delete;

    MeshManager &operator=(const MeshManager &other) = delete;
    MeshManager &operator=(MeshManager &&other) = delete;

    void registerMesh(const Mesh &&mesh, const std::string &name);
    [[nodiscard]] std::string registerMesh(const Mesh &&mesh);
    void unregisterMesh(const std::string &meshName);
    [[nodiscard]] bool meshRegistered(const MeshIdentifier &mId);

    void allocateMesh(const std::string &meshName);
    void deallocateMesh(const std::string &meshName);

    void bindMesh(const std::string &meshName);
    void unbindMesh();

    void cleanUpGracefully();

    const Mesh *getMesh(const std::string &meshName);

private:
    MeshManager() = default;

private:
    // TODO: rework not to use string as identifiers. Convenient but expensive
    std::unordered_map<MeshIdentifier, Mesh> _meshes;
    uint32_t _boundMesh = 0;
};
