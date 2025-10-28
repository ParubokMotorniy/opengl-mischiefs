#include "mesh.h"

#include <glad/glad.h>

#include <cstddef>

Mesh::Mesh(std::vector<Vertex> &&meshVertices, std::vector<uint32_t> &&meshIndices) : vertices(std::move(meshVertices)), indices(std::move(meshIndices)) {}

void Mesh::allocateMesh()
{
    if (id != 0)
        return;

    glGenVertexArrays(1, &id);
    glBindVertexArray(id);

    if (verticesSize() > 0 && indicesSize() > 0)
    {

        uint32_t VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, verticesSize(), vertices.data(), GL_STATIC_DRAW);

        uint32_t EBOOrange;
        glGenBuffers(1, &EBOOrange);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOOrange);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize(), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0); // space coords
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)offsetof(Vertex, texCoordinates));
        glEnableVertexAttribArray(1); // texture coords
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)offsetof(Vertex, normal));
        glEnableVertexAttribArray(2); // normals
    }
    
    glBindVertexArray(0);
}

void Mesh::deallocateMesh()
{
    if (id == 0)
        return;

    glDeleteVertexArrays(1, &id);
    id = 0;
}

void Mesh::bindMesh() const
{
    if (id == 0)
        return;
    glBindVertexArray(id);
}

bool Mesh::isAllocated() const noexcept
{
    return id != 0;
}

Mesh::~Mesh()
{
    deallocateMesh();
}

// size in bytes
uint32_t Mesh::verticesSize() const
{
    return vertices.size() * sizeof(Vertex);
}

uint32_t Mesh::numVertices() const
{
    return vertices.size();
}

// size in bytes
uint32_t Mesh::indicesSize() const
{
    return indices.size() * sizeof(uint32_t);
}

uint32_t Mesh::numIndices() const
{
    return indices.size();
}
