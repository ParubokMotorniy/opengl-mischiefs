#include "modelloader.h"

#include <iostream>

ModelLoader::ModelLoader() = default;

Model ModelLoader::loadModel(std::string const &path, bool flipTexturesOnLoad)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "Failed to load model wit assimp: " << importer.GetErrorString() << ". Model: " << path << std::endl;
        return Model();
    }

    if (flipTexturesOnLoad)
        stbi_set_flip_vertically_on_load(true);
    Utilities::ScopeGuard backFlipper([ifFlip = flipTexturesOnLoad]()
                                      {if(ifFlip)
                                        stbi_set_flip_vertically_on_load(false); });

    return processNode(scene->mRootNode, scene, path.substr(0, path.find_last_of('/')));
}

Model ModelLoader::processNode(aiNode *node, const aiScene *scene, const std::string &modelRoot)
{
    Model nodeModel;
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        nodeModel.modelComponents.push_back(processMesh(mesh, scene, modelRoot));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        nodeModel += processNode(node->mChildren[i], scene, modelRoot);
    }

    return nodeModel;
}

PrimitiveObject ModelLoader::processMesh(aiMesh *mesh, const aiScene *scene, const std::string &modelRoot)
{
    MeshIdentifier meshName = mesh->mName.length == 0 ? RandomNamer::instance()->getRandomName(10) : MeshIdentifier(mesh->mName.C_Str());

    if (!MeshManager::instance()->meshRegistered(meshName))
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            vertex.coordinates[0] = mesh->mVertices[i].x;
            vertex.coordinates[1] = mesh->mVertices[i].y;
            vertex.coordinates[2] = mesh->mVertices[i].z;

            // normals
            if (mesh->HasNormals())
            {
                vertex.normal[0] = mesh->mNormals[i].x;
                vertex.normal[1] = mesh->mNormals[i].y;
                vertex.normal[2] = mesh->mNormals[i].z;
            }
            // texture coordinates
            if (mesh->mTextureCoords[0])
            {
                vertex.texCoordinates[0] = mesh->mTextureCoords[0][i].x;
                vertex.texCoordinates[1] = mesh->mTextureCoords[0][i].y;
            }

            vertices.push_back(vertex);
        }

        // walk through each of the mesh's faces and retrieve indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        MeshManager::instance()->registerMesh(Mesh{std::move(vertices), std::move(indices)}, meshName);
    }

    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<TextureIdentifier> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, modelRoot);
    std::vector<TextureIdentifier> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, modelRoot);
    std::vector<TextureIdentifier> emissionMaps = loadMaterialTextures(material, aiTextureType_EMISSION_COLOR, modelRoot);

    const auto maxTexVecLength = std::ranges::max({diffuseMaps.size(), specularMaps.size(), emissionMaps.size()});
    diffuseMaps.resize(maxTexVecLength);
    specularMaps.resize(maxTexVecLength);
    emissionMaps.resize(maxTexVecLength);

    std::vector<Material> materials;
    materials.reserve(maxTexVecLength);
    for (int y = 0; y < maxTexVecLength; ++y)
    {
        materials.emplace_back(Material{diffuseMaps[y], specularMaps[y], emissionMaps[y]});
    }

    return PrimitiveObject{meshName, materials};
}

std::vector<TextureIdentifier> ModelLoader::loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &modelRoot)
{
    std::vector<TextureIdentifier> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        TextureIdentifier id = str.C_Str();
        if (!TextureManager::instance()->textureRegistered(id))
        {
            TextureManager::instance()->registerTexture((modelRoot + '/' + str.C_Str()).c_str(), str.C_Str());
        }
        textures.push_back(id);
    }
    return textures;
}
