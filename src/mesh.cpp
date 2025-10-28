#include "mesh.h"

#include <glad/glad.h>

#include <cstddef>

Mesh::Mesh(std::vector<Vertex> &&meshVertices, std::vector<uint32_t> &&meshIndices)
    : vertices(std::move(meshVertices)), indices(std::move(meshIndices))
{
}

void Mesh::allocateMesh()
{
    if (id != 0)
        return;

    glGenVertexArrays(1, &id);
    glBindVertexArray(id);

    if (verticesSize() > 0 && indicesSize() > 0)
    {
        glGenBuffers(1, &vId);
        glBindBuffer(GL_ARRAY_BUFFER, vId);
        glBufferData(GL_ARRAY_BUFFER, verticesSize(), vertices.data(), GL_STATIC_DRAW);

        uint32_t EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize(), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0); // space coords
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                              (void *)offsetof(Vertex, texCoordinates));
        glEnableVertexAttribArray(1); // texture coords
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                              (void *)offsetof(Vertex, normal));
        glEnableVertexAttribArray(2); // normals
    }

    glBindVertexArray(0);
}

void Mesh::deallocateMesh()
{
    if (id == 0)
        return;

    glDeleteBuffers(1, &vId);
    glDeleteVertexArrays(1, &id);
    glDeleteVertexArrays(1, &instancedId);

    id = 0;
    instancedId = 0;
}

void Mesh::bindMesh() const
{
    if (id == 0)
        return;
    glBindVertexArray(id);
}

// creates a separate VAO that allows binding instanced vertex attribute buffers to it later without
// polluting the non-instanced buffer
void Mesh::enableInstancing()
{
    if (id == 0)
        allocateMesh();

    if (instancedId != 0)
        return;

    glGenVertexArrays(1, &instancedId);
    glBindVertexArray(instancedId);

    if (verticesSize() > 0 && indicesSize() > 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vId); // shared

        uint32_t EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize(), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0); // space coords
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                              (void *)offsetof(Vertex, texCoordinates));
        glEnableVertexAttribArray(1); // texture coords
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                              (void *)offsetof(Vertex, normal));
        glEnableVertexAttribArray(2); // normals
    }

    glBindVertexArray(0);
}

bool Mesh::instancingEnabled() { return instancedId != 0; }

void Mesh::bindMeshInstanced()
{
    if (instancedId == 0)
        return;
    glBindVertexArray(instancedId);
}

uint32_t Mesh::standardArrayId() const noexcept { return id; }

uint32_t Mesh::instancedArrayId() const noexcept { return instancedId; }

bool Mesh::isAllocated() const noexcept { return id != 0; }

Mesh::~Mesh() { deallocateMesh(); }

// size in bytes
uint32_t Mesh::verticesSize() const { return vertices.size() * sizeof(Vertex); }

uint32_t Mesh::numVertices() const { return vertices.size(); }

// size in bytes
uint32_t Mesh::indicesSize() const { return indices.size() * sizeof(uint32_t); }

uint32_t Mesh::numIndices() const { return indices.size(); }
