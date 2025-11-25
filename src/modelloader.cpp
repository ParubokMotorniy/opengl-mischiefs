#include "modelloader.h"
#include "materialmanager.h"
#include "objectmanager.h"
#include "transformmanager.h"

#include <iostream>

ModelLoader::ModelLoader() = default;

GameObjectIdentifier ModelLoader::loadModel(std::string const &path, bool flipTexturesOnLoad,
                                            bool loadAsPbr)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path,
                                             aiProcess_Triangulate | aiProcess_GenSmoothNormals
                                                 | aiProcess_FlipUVs | aiProcess_CalcTangentSpace
                                                 | aiProcess_OptimizeMeshes
                                                 | aiProcess_OptimizeGraph);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "Failed to load model wit assimp: " << importer.GetErrorString()
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

    processNode(scene->mRootNode, scene, path.substr(0, path.find_last_of('/')), loadedObject,
                loadAsPbr);
    loadedObject.addComponent(
        Component(ComponentType::TRANSFORM,
                  TransformManager::instance()->registerNewTransform(loadedObject)));

    return loadedObject;
}

GameObjectIdentifier ModelLoader::processNode(aiNode *node, const aiScene *scene,
                                              const std::string &modelRoot,
                                              GameObjectIdentifier parentObject, bool loadAsPbr)
{
    GameObject &nodeObject = ObjectManager::instance()->getObject(
        ObjectManager::instance()->addObject());

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        nodeObject.addChildObject(processMesh(mesh, scene, modelRoot, loadAsPbr));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        nodeObject.addChildObject(
            processNode(node->mChildren[i], scene, modelRoot, nodeObject, loadAsPbr));
    }

    nodeObject.addComponent(
        Component(ComponentType::TRANSFORM,
                  TransformManager::instance()->registerNewTransform(nodeObject)));
    ObjectManager::instance()->getObject(parentObject).addChildObject(nodeObject);

    return nodeObject;
}

GameObjectIdentifier ModelLoader::processMesh(aiMesh *mesh, const aiScene *scene,
                                              const std::string &modelRoot, bool loadAsPbr)
{
    GameObject &meshContainer = ObjectManager::instance()->getObject(
        ObjectManager::instance()->addObject());

    MeshIdentifier meshId = InvalidIdentifier;

    if (meshId = MeshManager::instance()->meshRegistered(mesh->mName.C_Str());
        meshId == InvalidIdentifier)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<glm::vec3> tangents;

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

            if (loadAsPbr && mesh->HasTangentsAndBitangents())
            {
                // bitangents will be computed on the fly to reduce traffic
                tangents.emplace_back(mesh->mTangents[i].x, mesh->mTangents[i].y,
                                      mesh->mTangents[i].z);
            }
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        meshId = mesh->mName.length == 0
                     ? MeshManager::instance()
                           ->registerMesh(
                               Mesh{ std::move(vertices), std::move(indices), std::move(tangents) })
                           .second
                     : MeshManager::instance()->registerMesh(Mesh{ std::move(vertices),
                                                                   std::move(indices),
                                                                   std::move(tangents) },
                                                             std::string(mesh->mName.C_Str()));
    }

    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    TextureIdentifier diffuseMap = loadMaterialTextures(material,
                                                        { aiTextureType_DIFFUSE,
                                                          aiTextureType_BASE_COLOR },
                                                        modelRoot);
    TextureIdentifier specularMap = loadMaterialTextures(material, { aiTextureType_SPECULAR },
                                                         modelRoot);
    TextureIdentifier emissionMap = loadMaterialTextures(material, { aiTextureType_EMISSION_COLOR },
                                                         modelRoot);

    if (loadAsPbr)
    {
        TextureIdentifier albedo = loadMaterialTextures(material,
                                                        { aiTextureType_BASE_COLOR,
                                                          aiTextureType_DIFFUSE },
                                                        modelRoot);
        TextureIdentifier normal = loadMaterialTextures(material,
                                                        { aiTextureType_NORMAL_CAMERA,
                                                          aiTextureType_NORMALS,
                                                          aiTextureType_HEIGHT },
                                                        modelRoot, false);
        TextureIdentifier roughness = loadMaterialTextures(material,
                                                           { aiTextureType_DIFFUSE_ROUGHNESS },
                                                           modelRoot, false);
        TextureIdentifier ambientOcclusion
            = loadMaterialTextures(material, { aiTextureType_AMBIENT_OCCLUSION }, modelRoot, false);
        TextureIdentifier metalness = loadMaterialTextures(material,
                                                           { aiTextureType_METALNESS,
                                                             aiTextureType_SPECULAR },
                                                           modelRoot, false);

        if (metalness != InvalidIdentifier && roughness != InvalidIdentifier
            && normal != InvalidIdentifier)
        {
            const auto [mName,
                        mi] = MaterialManager<PbrMaterial, ComponentType::PBR_MATERIAL>::instance()
                                  ->registerMaterial(PbrMaterial{
                                      albedo, normal, roughness, metalness,
                                      ambientOcclusion != InvalidIdentifier
                                          ? ambientOcclusion
                                          : TextureManager::instance()->textureRegistered(
                                                "white") });
            meshContainer.addComponent(Component(ComponentType::PBR_MATERIAL, mi));
        }
        else
        {
            std::cerr << "The requested model does not include the necessary PBR textures!"
                      << std::endl;
        }
    }

    const auto [mName,
                mi] = MaterialManager<BasicMaterial, ComponentType::BASIC_MATERIAL>::instance()
                          ->registerMaterial(BasicMaterial{
                              diffuseMap,
                              specularMap != InvalidIdentifier
                                  ? specularMap
                                  : TextureManager::instance()->textureRegistered("black"),
                              emissionMap != InvalidIdentifier
                                  ? emissionMap
                                  : TextureManager::instance()->textureRegistered("black") });

    meshContainer.addComponent(Component(ComponentType::MESH, meshId));
    meshContainer.addComponent(Component(ComponentType::BASIC_MATERIAL, mi));
    meshContainer.addComponent(
        Component(ComponentType::TRANSFORM,
                  TransformManager::instance()->registerNewTransform(meshContainer)));

    return meshContainer;
}

// allows only one texture of a particular type
TextureIdentifier ModelLoader::loadMaterialTextures(
    aiMaterial *mat, const std::initializer_list<aiTextureType> &types,
    const std::string &modelRoot, bool loadAsSrgb)
{
    TextureIdentifier texture = InvalidIdentifier;

    for (const auto type : types)
    {
        for (unsigned int i = 0; i < mat->GetTextureCount(type) && i < 1;
             i++) // limit to one texture currently
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            const std::string texName = str.C_Str();
            if (texture = TextureManager::instance()->textureRegistered(texName);
                texture == InvalidIdentifier)
            {
                texture = TextureManager::instance()
                              ->registerTexture((modelRoot + '/' + str.C_Str()).c_str(), texName);
                TextureManager::instance()->getTexture(texture)->setUseSrgb(loadAsSrgb);
            }
            if (texture != InvalidIdentifier)
                return texture;
        }
    }
    return texture;
}
