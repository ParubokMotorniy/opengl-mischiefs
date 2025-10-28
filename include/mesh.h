#pragma once
#include "material.h"

#include <vector>
#include <cstdint>

#pragma pack(push, 1)
struct Vertex
{
    float coordinates[3] = {0.0f, 0.0f, 0.0f};
    float texCoordinates[2] = {0.0f, 0.0f};
    float normal[3] = {0.0f, 0.0f, 0.0f};
};
#pragma pack(pop)

struct Mesh
{
    Mesh() = default;
    Mesh(std::vector<Vertex> &&meshVertices, std::vector<uint32_t> &&meshIndices);
    ~Mesh();

    bool isAllocated() const noexcept;
    void allocateMesh();
    void deallocateMesh();

    void bindMesh() const;

    void enableInstancing();
    bool instancingEnabled();
    void bindMeshInstanced();

    uint32_t standardArrayId() const noexcept;
    uint32_t instancedArrayId() const noexcept;

    // size in bytes
    uint32_t verticesSize() const;

    uint32_t numVertices() const;

    // size in bytes
    uint32_t indicesSize() const;

    uint32_t numIndices() const;

private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    uint32_t vId = 0; //vbo
    uint32_t id = 0; //vao

    uint32_t instancedId = 0;
};
