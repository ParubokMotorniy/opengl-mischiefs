#pragma once

#include <string>

namespace ResourceManagement
{
// each file is treated as a texture
void loadTextures(const std::string &directoryPath);

// in each directory, treats all regular files as textures of a single cubemap. Then recursively
// processes the subdirectories
void loadCubemaps(const std::string &directoryPath);

void loadModels(const std::string &directoryPath);
}; // namespace ResourceManagement