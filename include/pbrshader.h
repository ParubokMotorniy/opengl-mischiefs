#pragma once

#include "instancedshader.h"

#include <string>

class PbrShader : public InstancedShader<PbrMaterial>
{
public:
    PbrShader(const char *vertexPath, const char *fragmentPath);
    void runShader() override;

protected:
    std::vector<InstancedDataGenerator> getDataGenerators() override;
};
