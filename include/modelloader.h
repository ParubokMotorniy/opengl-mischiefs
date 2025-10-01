#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>

#include "texturemanager.h"
#include "meshmanager.h"
#include "material.h"
#include "randomnamer.h"
#include "utils.h"
#include "singleton.h"
#include "object.h"

// shamefully adapted from learnopengl

//TODO: add mesh transform loading
class ModelLoader : public SystemSingleton<ModelLoader>
{
public:
    friend class SystemSingleton;

    GameObjectIdentifier loadModel(std::string const &path, bool flipTexturesOnLoad = false);

private:
    ModelLoader();

    GameObjectIdentifier processNode(aiNode *node, const aiScene *scene, const std::string &modelRoot, GameObjectIdentifier parentObject);
    GameObjectIdentifier processMesh(aiMesh *mesh, const aiScene *scene, const std::string &modelRoot);
    TextureIdentifier loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &modelRoot);
};
