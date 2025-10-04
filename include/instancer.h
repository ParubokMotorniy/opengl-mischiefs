#pragma once

#include "singleton.h"
#include "types.h"

#include "glm/glm.hpp"
#include <glad/glad.h>

#include <span>
#include <functional>
#include <vector>
#include <algorithm>
#include <numeric>

using InstancedDataGeneratorFunc = std::function<void(int8_t *, GameObjectIdentifier)>;
struct InstancedDataGenerator
{
    size_t dataByteSize = 0; //total bytes generated
    size_t dataUnitSize = 0; //total number of data subunits wrt to `dataType` (like GL_FLOAT or GL_INT)
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
    uint32_t instanceData(const std::span<const GameObjectIdentifier, std::dynamic_extent> &instancedObjects, const std::vector<InstancedDataGenerator> &generators, const uint32_t targetVertexArray)
    {
        const size_t dataSizePerObject = std::accumulate(generators.cbegin(), generators.cend(), (size_t)0, [](size_t rSum, const InstancedDataGenerator &gen) -> size_t
                                                         { return rSum + gen.dataByteSize; });
        const size_t totalBufferLength = instancedObjects.size() * dataSizePerObject;

        std::vector<int8_t> buffer;
        buffer.resize(totalBufferLength);

        size_t bytesAccumulated = 0;
        for (const GameObjectIdentifier id : instancedObjects)
        {
            for (size_t g = 0; g < generators.size(); ++g)
            {
                generators[g].func(buffer.data() + bytesAccumulated, id);
                bytesAccumulated += generators[g].dataByteSize;
            }
        }
        glBindVertexArray(targetVertexArray);

        uint32_t vertexBuffer;
        glGenBuffers(1, &vertexBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, totalBufferLength, buffer.data(), GL_STATIC_DRAW);

        size_t bytesMapped = 0;
        for (const InstancedDataGenerator &gen : generators)
        {
            glVertexAttribPointer(gen.location, gen.dataUnitSize, gen.dataType, gen.normalize ? GL_TRUE : GL_FALSE, dataSizePerObject, (void *)bytesMapped);
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
