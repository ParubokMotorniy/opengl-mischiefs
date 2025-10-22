#include "modelloader.h"
#include "materialmanager.h"
#include "objectmanager.h"
#include "transformmanager.h"

#include <iostream>

ModelLoader::ModelLoader() = default;

GameObjectIdentifier ModelLoader::loadModel(std::string const &path, bool flipTexturesOnLoad)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path,
                                             aiProcess_Triangulate | aiProcess_GenSmoothNormals
                                                 | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
                                                 | aiProcess_OptimizeMeshes);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "Failed to load model wit assimp: " << importer.GetErrorString()
                  << ". Model: " << path << std::endl;
        return InvalidIdentifier;
    }

    if (flipTexturesOnLoad)
        stbi_set_flip_vertically_on_load(true);
    Utilities::ScopeGuard backFlipper([ifFlip = flipTexturesOnLoad]() {
        if (ifFlip)
            stbi_set_flip_vertically_on_load(false);
    });

    GameObject &loadedObject = ObjectManager::instance()->getObject(
        ObjectManager::instance()->addObject());

    processNode(scene->mRootNode, scene, path.substr(0, path.find_last_of('/')), loadedObject);
    loadedObject.addComponent(
        Component(ComponentType::TRANSFORM,
                  TransformManager::instance()->registerNewTransform(loadedObject)));

    return loadedObject;
}

GameObjectIdentifier ModelLoader::processNode(aiNode *node, const aiScene *scene,
                                              const std::string &modelRoot,
                                              GameObjectIdentifier parentObject)
{
    GameObject &nodeObject = ObjectManager::instance()->getObject(
        ObjectManager::instance()->addObject());

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        nodeObject.addChildObject(processMesh(mesh, scene, modelRoot));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        nodeObject.addChildObject(processNode(node->mChildren[i], scene, modelRoot, nodeObject));
    }

    nodeObject.addComponent(
        Component(ComponentType::TRANSFORM,
                  TransformManager::instance()->registerNewTransform(nodeObject)));
    ObjectManager::instance()->getObject(parentObject).addChildObject(nodeObject);

    return nodeObject;
}

GameObjectIdentifier ModelLoader::processMesh(aiMesh *mesh, const aiScene *scene,
                                              const std::string &modelRoot)
{
    GameObject &meshContainer = ObjectManager::instance()->getObject(
        ObjectManager::instance()->addObject());

    MeshIdentifier meshId = InvalidIdentifier;

    if (meshId = MeshManager::instance()->meshRegistered(mesh->mName.C_Str());
        meshId == InvalidIdentifier)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

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

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        meshId = mesh->mName.length == 0
                     ? MeshManager::instance()
                           ->registerMesh(Mesh{ std::move(vertices), std::move(indices) })
                           .second
                     : MeshManager::instance()->registerMesh(Mesh{ std::move(vertices),
                                                                   std::move(indices) },
                                                             std::string(mesh->mName.C_Str()));
    }

    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    TextureIdentifier diffuseMap = loadMaterialTextures(material, aiTextureType_DIFFUSE, modelRoot);
    TextureIdentifier specularMap = loadMaterialTextures(material, aiTextureType_SPECULAR,
                                                         modelRoot);
    TextureIdentifier emissionMap = loadMaterialTextures(material, aiTextureType_EMISSION_COLOR,
                                                         modelRoot);

    const auto [mName,
                mi] = MaterialManager<BasicMaterial, ComponentType::BASIC_MATERIAL>::instance()
                          ->registerMaterial(BasicMaterial{ diffuseMap, specularMap, emissionMap });

    meshContainer.addComponent(Component(ComponentType::MESH, meshId));
    meshContainer.addComponent(Component(ComponentType::BASIC_MATERIAL, mi));
    meshContainer.addComponent(
        Component(ComponentType::TRANSFORM,
                  TransformManager::instance()->registerNewTransform(meshContainer)));
    return meshContainer;
}

// allows only one texture of a particular type
TextureIdentifier ModelLoader::loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                                    const std::string &modelRoot)
{
    TextureIdentifier texture = InvalidIdentifier;

    for (unsigned int i = 0; i < mat->GetTextureCount(type) && i < 1; i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        const std::string texName = str.C_Str();
        if (texture = TextureManager::instance()->textureRegistered(texName);
            texture == InvalidIdentifier)
        {
            texture = TextureManager::instance()
                          ->registerTexture((modelRoot + '/' + str.C_Str()).c_str(), texName);
        }
    }

    return texture;
}
