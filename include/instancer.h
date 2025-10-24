#pragma once

#include "singleton.h"
#include "types.h"
#include "utils.h"

#include "glm/glm.hpp"
#include <glad/glad.h>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <numeric>
#include <span>
#include <stdlib.h>
#include <vector>

using InstancedDataGeneratorFunc = std::function<void(void *, GameObjectIdentifier)>;
struct InstancedDataGenerator
{
    size_t dataByteSize = 0; // total bytes generated
    size_t dataUnitSize
        = 0; // total number of data subunits wrt to `dataType` (like GL_FLOAT or GL_INT)
    size_t location = 0;
    int dataType = 0;
    bool normalize = false;
    InstancedDataGeneratorFunc func;
};

class Instancer : public SystemSingleton<Instancer>
{
public:
    friend class SystemSingleton<Instancer>;

    void updateInstancedData(
        const GLuint instancedElementBuffer, const std::vector<InstancedDataGenerator> &generators,
        const std::vector<std::pair<GameObjectIdentifier, uint32_t>> &indicesOfUpdatedBuffers)
    {
        const size_t dataSizePerObject
            = std::accumulate(generators.cbegin(), generators.cend(), (size_t)0,
                              [](size_t rSum, const InstancedDataGenerator &gen) -> size_t {
                                  return rSum + gen.dataByteSize;
                              });

#ifdef LINUX
        void *buffer = std::aligned_alloc(16, dataSizePerObject);
        const auto cleanUp = Utilities::ScopeGuard([buffer]() { std::free(buffer); });
#endif

#ifdef WINDOWS
        void *buffer = _aligned_malloc(dataSizePerObject, 16);
        const auto cleanUp = Utilities::ScopeGuard([buffer]() { _aligned_free(buffer); });
#endif

        glBindBuffer(GL_ARRAY_BUFFER, instancedElementBuffer);

        for (const auto [gId, gIdx] : indicesOfUpdatedBuffers)
        {
            for (size_t g = 0, bytesAccumulated = 0; g < generators.size(); ++g)
            {
                generators[g].func((int8_t *)(buffer) + bytesAccumulated, gId);
                bytesAccumulated += generators[g].dataByteSize;
            }
            //TODO: try implementing through buffer mapping
            glBufferSubData(GL_ARRAY_BUFFER, gIdx * dataSizePerObject, dataSizePerObject, buffer);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // TODO: try to make use of compile-time polymorphism of the generators
    GLuint instanceData(
        const std::span<const GameObjectIdentifier, std::dynamic_extent> &instancedObjects,
        const std::vector<InstancedDataGenerator> &generators, const uint32_t targetVertexArray)
    {
        const size_t dataSizePerObject
            = std::accumulate(generators.cbegin(), generators.cend(), (size_t)0,
                              [](size_t rSum, const InstancedDataGenerator &gen) -> size_t {
                                  return rSum + gen.dataByteSize;
                              });
        const size_t totalBufferLength = instancedObjects.size() * dataSizePerObject;

#ifdef LINUX
        void *buffer = std::aligned_alloc(16, totalBufferLength);
        const auto cleanUp = Utilities::ScopeGuard([buffer]() { std::free(buffer); });
#endif

#ifdef WINDOWS
        void *buffer = _aligned_malloc(totalBufferLength, 16);
        const auto cleanUp = Utilities::ScopeGuard([buffer]() { _aligned_free(buffer); });
#endif

        size_t bytesAccumulated = 0;
        for (const GameObjectIdentifier id : instancedObjects)
        {
            for (size_t g = 0; g < generators.size(); ++g)
            {
                generators[g].func((int8_t *)(buffer) + bytesAccumulated, id);
                bytesAccumulated += generators[g].dataByteSize;
            }
        }
        glBindVertexArray(targetVertexArray);

        GLuint vertexBuffer;
        glGenBuffers(1, &vertexBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, totalBufferLength, buffer, GL_DYNAMIC_DRAW);

        size_t bytesMapped = 0;
        for (const InstancedDataGenerator &gen : generators)
        {
            if (gen.dataType == GL_INT || gen.dataType == GL_UNSIGNED_INT)
                glVertexAttribIPointer(gen.location, gen.dataUnitSize, gen.dataType,
                                       dataSizePerObject, (void *)bytesMapped);
            else
            {
                glVertexAttribPointer(gen.location, gen.dataUnitSize, gen.dataType,
                                      gen.normalize ? GL_TRUE : GL_FALSE, dataSizePerObject,
                                      (void *)bytesMapped);
            }
            glEnableVertexAttribArray(gen.location);
            glVertexAttribDivisor(gen.location, 1);
            bytesMapped += gen.dataByteSize;
        }

        glBindVertexArray(0);
        return vertexBuffer;
    }

private:
    Instancer() = default;
};
