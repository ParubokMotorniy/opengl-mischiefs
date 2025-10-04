#pragma once

#include "singleton.h"
#include "types.h"
#include "utils.h"

#include "glm/glm.hpp"
#include <glad/glad.h>

#include <algorithm>
#include <functional>
#include <numeric>
#include <span>
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

    // TODO: try to make use of compile-time polymorphism of the generators
    uint32_t instanceData(
        const std::span<const GameObjectIdentifier, std::dynamic_extent> &instancedObjects,
        const std::vector<InstancedDataGenerator> &generators, const uint32_t targetVertexArray)
    {
        const size_t dataSizePerObject
            = std::accumulate(generators.cbegin(), generators.cend(), (size_t)0,
                              [](size_t rSum, const InstancedDataGenerator &gen) -> size_t {
                                  return rSum + gen.dataByteSize;
                              });
        const size_t totalBufferLength = instancedObjects.size() * dataSizePerObject;

        void *buffer = std::aligned_alloc(16, totalBufferLength);
        const auto cleanUp = Utilities::ScopeGuard([buffer]() { std::free(buffer); });

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

        uint32_t vertexBuffer;
        glGenBuffers(1, &vertexBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, totalBufferLength, buffer, GL_STATIC_DRAW);

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
