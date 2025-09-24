#pragma once

#include "model.h"

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

// shamefully adapted from learnopengl

class ModelLoader
{
public:
    static ModelLoader *instance();

    ModelLoader(const ModelLoader &other) = delete;
    ModelLoader(ModelLoader &&other) = delete;

    ModelLoader &operator=(const ModelLoader &other) = delete;
    ModelLoader &operator=(ModelLoader &&other) = delete;

    Model loadModel(std::string const &path, bool flipTexturesOnLoad = false);

private:
    ModelLoader();

    Model processNode(aiNode *node, const aiScene *scene, const std::string &modelRoot);
    PrimitiveObject processMesh(aiMesh *mesh, const aiScene *scene, const std::string &modelRoot);
    std::vector<TextureIdentifier> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &modelRoot);
};
