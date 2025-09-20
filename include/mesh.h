#pragma once

#include <vector>

#pragma pack(push, 1)
struct Vertex
{
    float coordinates[3];
    float texCoordinates[2];
    float normal[3];
};
#pragma pack(pop)

struct Mesh
{
    Mesh(std::vector<Vertex> &&meshVertices, std::vector<uint> &&meshIndices) : vertices(std::move(meshVertices)), indices(std::move(meshIndices)) {}

    void allocateMesh()
    {
        if(id != 0)
            return;

        glGenVertexArrays(1, &id);
        glBindVertexArray(id);

        uint VBO;
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, verticesSize(), vertices.data(), GL_STATIC_DRAW);

        uint EBOOrange;
        glGenBuffers(1, &EBOOrange);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOOrange);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize(), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0); // space coords
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1); // texture coords
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
        glEnableVertexAttribArray(2); // normals

        glBindVertexArray(0);
    }

    void deallocateMesh()
    {
        if (id == 0)
            return;

        glDeleteVertexArrays(1, &id);
        id = 0;
    }

    void bindMesh() const
    {
        if (id == 0)
            return;
        glBindVertexArray(id);
    }

    bool isAllocated() const noexcept
    {
        return id != 0;
    }

    ~Mesh()
    {
        deallocateMesh();
    }

    operator int()
    {
        return id;
    }

    //size in bytes
    uint verticesSize() const
    {
        return vertices.size() * sizeof(Vertex);
    }

    uint numVertices() const 
    {
        return vertices.size();
    }

    //size in bytes
    uint indicesSize() const
    {
        return indices.size() * sizeof(uint);
    }

    uint numIndices() const 
    {
        return indices.size();
    }

private:
    std::vector<Vertex> vertices;
    std::vector<uint> indices;
    uint id = 0;
};
