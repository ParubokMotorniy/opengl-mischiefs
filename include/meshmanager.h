#pragma once

#include "mesh.h"
#include "singleton.h"
#include "types.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class MeshManager : public SystemSingleton<MeshManager>
{
public:
    friend class SystemSingleton;
    using NamedMesh = NamedComponent<Mesh>;

    MeshIdentifier registerMesh(const Mesh &&mesh, const std::string &name);
    [[nodiscard]] std::pair<std::string, MeshIdentifier> registerMesh(const Mesh &&mesh);

    void unregisterMesh(MeshIdentifier id);
    [[nodiscard]] MeshIdentifier meshRegistered(const std::string &meshName);

    void allocateMesh(MeshIdentifier id);
    void deallocateMesh(MeshIdentifier id);

    void bindMesh(MeshIdentifier id);
    void bindMeshInstanced(MeshIdentifier id);
    void unbindMesh();
    MeshIdentifier getDummyMesh() const;

    void enableMeshInstancing(MeshIdentifier id);

    void cleanUpGracefully();

    const Mesh *getMesh(MeshIdentifier id);

private:
    MeshManager() { registerMesh(Mesh(), "dummy_mesh"); }

private:
    MeshIdentifier _identifiers = 0;
    std::unordered_map<MeshIdentifier, NamedMesh> _meshes;
    MeshIdentifier _boundMesh = 0;

    MeshIdentifier _dummyMesh = InvalidIdentifier;
};
