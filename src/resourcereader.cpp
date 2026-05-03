#include "resourcereader.h"

#include "modelloader.h"
#include "texture3d.h"
#include "texturemanager.h"
#include "texturemanager3d.h"

#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
constexpr std::array<const char *, 2> supportedExtensions = { ".obj", ".gltf" };
}

namespace ResourceManagement
{
// each file is treated as a texture
void loadTextures(const std::string &directoryPath)
{
    namespace fs = std::filesystem;
    for (const fs::directory_entry &dirEntry : fs::recursive_directory_iterator(directoryPath))
    {
        if (!dirEntry.is_regular_file() || !dirEntry.exists())
            continue;
        const auto &textureName = dirEntry.path().stem();
        const auto &texturePath = dirEntry.path();

        TextureManager::instance()->registerTexture(texturePath.c_str(), textureName);
    }
}

// in each directory, treats all regular files as textures of a single cubemap. Then recursively
// processes the subdirectories
void loadCubemaps(const std::string &directoryPath)
{
    namespace fs = std::filesystem;

    std::array<std::string, Cubemap::Cubemap::NumberOfFaces> cubemapFaces;
    int discoveredFaces = 0;
    for (const fs::directory_entry &dirEntry : fs::directory_iterator(directoryPath))
    {
        if (!dirEntry.exists())
            continue;

        if (dirEntry.is_directory())
        {
            loadCubemaps(dirEntry.path());
            continue;
        }

        if (dirEntry.is_regular_file() && discoveredFaces < Cubemap::Cubemap::NumberOfFaces)
        {
            cubemapFaces[discoveredFaces++] = dirEntry.path();
            continue;
        }
    }

    if (cubemapFaces.size() < Cubemap::Cubemap::NumberOfFaces)
        return;

    std::sort(cubemapFaces.begin(), cubemapFaces.end(),
              [](const std::string &face1, const std::string &face2) {
                  return face1[0] < face2[0];
              });

    CubemapManager::instance()->registerTexture(cubemapFaces,
                                                (fs::path(cubemapFaces[0]).stem().c_str()
                                                 + 1 /*+1 to skip the leading digit*/));
}

void loadModels(const std::string &directoryPath)
{
    namespace fs = std::filesystem;
    for (const fs::directory_entry &dirEntry : fs::recursive_directory_iterator(directoryPath))
    {
        if (!dirEntry.is_regular_file() || !dirEntry.exists()
            || std::find(supportedExtensions.begin(), supportedExtensions.end(),
                         dirEntry.path().extension())
                   == supportedExtensions.end())
            continue;
        const auto &textureName = dirEntry.path().stem();

        // at startup we can allow to check PBR materials of every model
        ModelLoader::instance()->loadModel(dirEntry.path(), false, true);
    }
}
}; // namespace ResourceManagement