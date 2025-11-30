#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <string>
#include <vector>

#include "material.h"
#include "meshmanager.h"
#include "object.h"
#include "randomnamer.h"
#include "singleton.h"
#include "texturemanager.h"
#include "utils.h"

// shamefully adapted from learnopengl

// TODO: add mesh transform loading
class ModelLoader : public SystemSingleton<ModelLoader>
{
public:
    friend class SystemSingleton;

    GameObjectIdentifier loadModel(std::string const &path, bool flipTexturesOnLoad = false,
                                   bool loadAsPbr = false);

private:
    ModelLoader();

    GameObjectIdentifier processNode(aiNode *node, const aiScene *scene,
                                     const std::string &modelRoot,
                                     GameObjectIdentifier parentObject, bool loadAsPbr);
    GameObjectIdentifier processMesh(aiMesh *mesh, const aiScene *scene,
                                     const std::string &modelRoot, bool loadAsPbr);
    
    TextureIdentifier loadMaterialTextures(aiMaterial *mat, const std::initializer_list<aiTextureType> &types,
                                           const std::string &modelRoot, bool loadAsSrgb = true);
};
