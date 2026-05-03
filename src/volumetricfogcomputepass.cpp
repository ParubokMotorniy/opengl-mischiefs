#include "volumetricfogcomputepass.h"
#include "camera.h"
#include "lightmanager.h"
#include "texturemanager.h"
#include "timemanager.h"

#include "glad/glad.h"
#include "imgui/imgui.h"

#include <iostream>

namespace
{
float fogDensity = 1.0f;
float sphereRadius = 10.0f;

float transmittance = 0.75f;
float lod0ShellWidth = 5.0f;
float lodSelectionFunctionPower = 1.4f;

int stepsPerVolume = 10;
float maxMarchDistance = 50.0f;

float noiseInfluence = 0.75f;

glm::vec3 lightAbsorbtion = glm::vec3(1.0f);
glm::vec3 fogColor = glm::vec3(0.984f, 0.773f, 0.765f);

constexpr glm::uvec3 groupSize = glm::uvec3(32, 16, 1);
constexpr size_t screenDiscretizationResoutionX = 1920;
constexpr size_t screenDiscretizationResoutionY = 1080;

constexpr float clearPosition[1] = { 1.0f };
constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

} // namespace

VolumetricFogPass::VolumetricFogPass()
{
    _fogSphereShader.initializeShaderProgram();

    const auto colorImageCreator = []() {
        unsigned int texture;

        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, screenDiscretizationResoutionX,
                     screenDiscretizationResoutionY, 0, GL_RGBA, GL_FLOAT, NULL);

        return TextureManager::instance()->registerTexture(texture);
    };

    const auto depthImageCreator = []() {
        unsigned int texture;

        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, screenDiscretizationResoutionX,
                     screenDiscretizationResoutionY, 0, GL_RED, GL_FLOAT, NULL);

        return TextureManager::instance()->registerTexture(texture);
    };

    _readPair.colorTexture = colorImageCreator();
    _readPair.positionTexture = depthImageCreator();

    _writePair.colorTexture = colorImageCreator();
    _writePair.positionTexture = depthImageCreator();

    {
        unsigned int fogTexture;
        glGenTextures(1, &fogTexture);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, fogTexture);

        int width, height, numChannels;
        auto *imageData = stbi_load(ENGINE_TEXTURES "/fog.png", &width, &height, &numChannels, 0);
        assert(numChannels > 0 && imageData != nullptr);

        const int slicesPerDimension = 4;
        const int numSlices = slicesPerDimension * slicesPerDimension;
        const int texelsPerY = (height / slicesPerDimension);
        const int texelsPerX = (width / slicesPerDimension);
        const int texelsPerImage = texelsPerX * texelsPerY;
        std::vector<unsigned char> atlasedTexture;
        atlasedTexture.resize(width * height * numChannels);

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const int nextTexelIdx = (y * width + x) * numChannels;

                const int imageIdxX = x / texelsPerX;
                const int imageIdxY = y / texelsPerY;
                const int imageIdx = imageIdxY * slicesPerDimension + imageIdxX;

                const int targetIdx = ((imageIdx * texelsPerImage)
                                       + ((y % texelsPerY) * texelsPerX + (x % texelsPerX)))
                                      * numChannels;

                for (int c = 0; c < numChannels; ++c)
                {
                    const auto nextTexel = imageData[nextTexelIdx + c];
                    atlasedTexture[targetIdx + c] = nextTexel;
                }
            }
        }

        GLenum format = 0;
        GLenum internalFormat = 0;
        if (numChannels == 1)
        {
            format = GL_RED;
            internalFormat = GL_RED;
        }
        else if (numChannels == 3)
        {
            format = GL_RGB;
            internalFormat = GL_SRGB;
        }
        else if (numChannels == 4)
        {
            format = GL_RGBA;
            internalFormat = GL_SRGB_ALPHA;
        }

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

        const float borderColor[4] = { 0.0, 0.0, 0.0, 0.0 };
        glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, texelsPerX, texelsPerY, numSlices, 0, format,
                     GL_UNSIGNED_BYTE, atlasedTexture.data());
        glGenerateMipmap(GL_TEXTURE_3D);
        glBindTexture(GL_TEXTURE_3D, 0);

        _numMipLeves = glm::floor(glm::log2(static_cast<float>(texelsPerX))) + 1;
        stbi_image_free(imageData);

        _fogTexture = TextureManager::instance()->registerTexture(fogTexture);
    }
}

void VolumetricFogPass::runPass()
{
    {
        {
            const auto [viewportX, viewportY] = _currentWindow->currentWindowDimensions();
            ImGui::SetNextWindowPos(ImVec2(viewportX * 0.05, viewportY * 0.45), ImGuiCond_Always,
                                    ImVec2(0.0f, 0.5f));
        }
        ImGui::Begin("Fog parameters", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Technical mumbo-jumbo");
        ImGui::Separator();
        ImGui::SliderFloat("LOD 0 shell width", &lod0ShellWidth, 1.0f, 50.0f);
        ImGui::SliderInt("Steps per volume", &stepsPerVolume, 5, 100);
        ImGui::SliderFloat("LOD function power", &lodSelectionFunctionPower, 1.2f, 2.0f);
        ImGui::SliderFloat("Max march distance", &maxMarchDistance, 25.0f, 150.0f);
        ImGui::Separator();

        ImGui::Text("Prettiness thingies");
        ImGui::Separator();
        ImGui::SliderFloat("Fog density", &fogDensity, 0.001f, 20.0f);
        ImGui::SliderFloat("Fog sphere radius", &sphereRadius, 1.0f, 50.0f);
        ImGui::SliderFloat("Noise influence", &noiseInfluence, 0.05f, 1.0f);
        ImGui::SliderFloat("Transmittance", &transmittance, 0.001f, 1.0f);
        ImGui::SliderFloat3("Light absorbtion", (float *)&lightAbsorbtion, 0.01f, 1.0f);
        ImGui::SliderFloat3("Fog color", (float *)&fogColor, 0.01f, 1.0f);
        ImGui::Separator();

        ImGui::End();
    }

    {
        const auto currentViewMatrix = _currentCamera->getViewMatrix();
        const auto currentProjectionMatrix = _currentCamera->projectionMatrix();

        const glm::vec3 sphereViewPosition = currentViewMatrix * glm::vec4(5.0f, 5.0f, 5.0f, 1.0f);
        const float zDistanceToSphereCenter = glm::abs(sphereViewPosition.z);

        _volumeIsOutOfSight = maxMarchDistance < zDistanceToSphereCenter;

        // the cheapest sphere is the one we don't render :)
        if (_volumeIsOutOfSight)
        {
            {
                const auto colorImageIdentifier = getCurrentReadTarget().colorTexture;
                const int colorImageHandle = *TextureManager::instance()->getTexture(
                    colorImageIdentifier);
                glClearTexImage(colorImageHandle, 0, GL_RGBA, GL_FLOAT, clearColor);
            }

            {
                const auto positionImageIdentifier = getCurrentReadTarget().positionTexture;
                const int positionImageHandle = *TextureManager::instance()->getTexture(
                    positionImageIdentifier);

                glClearTexImage(positionImageHandle, 0, GL_RED, GL_FLOAT, clearPosition);
            }

            {
                const auto colorImageIdentifier = getCurrentWriteTarget().colorTexture;
                const int colorImageHandle = *TextureManager::instance()->getTexture(
                    colorImageIdentifier);
                glClearTexImage(colorImageHandle, 0, GL_RGBA, GL_FLOAT, clearColor);
            }

            {
                const auto positionImageIdentifier = getCurrentWriteTarget().positionTexture;
                const int positionImageHandle = *TextureManager::instance()->getTexture(
                    positionImageIdentifier);

                glClearTexImage(positionImageHandle, 0, GL_RED, GL_FLOAT, clearPosition);
            }

            return;
        }

        // gives the straight distance to the nearest bounding plane of the sphere
        const float straightDistanceToSphere = zDistanceToSphereCenter < sphereRadius
                                                   ? zDistanceToSphereCenter
                                                   : zDistanceToSphereCenter - sphereRadius;
        const float currentTime = TimeManager::instance()->getTime();
        const float sphereDiameter = 2.0f * sphereRadius;

        // LOD selection
        const float distanceRatio = glm::clamp((straightDistanceToSphere - lod0ShellWidth)
                                                   / (maxMarchDistance - sphereDiameter),
                                               0.0f, 1.0f);

        const float LODSlopeValue = std::pow(distanceRatio, lodSelectionFunctionPower)
                                    * _numMipLeves;

        const int bottomMip = glm::floor(LODSlopeValue);
        const int ceilingMip = glm::ceil(LODSlopeValue);
        const float mixingCoefficient = LODSlopeValue - bottomMip;

        // step size selection
        const float marchStepSize
            = glm::max(glm::min(2.0f * sphereRadius, maxMarchDistance - straightDistanceToSphere),
                       0.0001f)
              / stepsPerVolume; // the step size decreases as the sphere goes out of sight so as not
                                // to produce rapid visual artifacts

        _fogSphereShader.use();
        _fogSphereShader.setInt("resolutionX", screenDiscretizationResoutionX);
        _fogSphereShader.setInt("resolutionY", screenDiscretizationResoutionY);

        _fogSphereShader.setInt("currentMipLevelFloor", bottomMip);
        _fogSphereShader.setInt("currentMipLevelCeiling", ceilingMip);
        _fogSphereShader.setFloat("lodMixingCoefficient", mixingCoefficient);

        _fogSphereShader.setFloat("marchStepSize", marchStepSize);
        _fogSphereShader.setFloat("maxMarchDistance", maxMarchDistance);
        _fogSphereShader.setMatrix3("fogVolumeWorldToModelRotation",
                                    glm::inverse(
                                        glm::rotate(glm::identity<glm::mat4>(),
                                                    glm::radians(
                                                        currentTime
                                                        - (float)(static_cast<int>(currentTime) / 2)
                                                              * 360.0f),
                                                    glm::vec3(1.0f, 1.0f, 1.0f))));
        _fogSphereShader.setMatrix4("inverseViewMatrix", glm::inverse(currentViewMatrix));

        _fogSphereShader.setMatrix4("viewMatrix", currentViewMatrix);
        _fogSphereShader.setMatrix4("projectionMatrix", currentProjectionMatrix);
        _fogSphereShader.setMatrix4("clipToView", glm::inverse(currentProjectionMatrix));

        _fogSphereShader.setVec3("viewSpherePos", sphereViewPosition);
        _fogSphereShader.setFloat("sphereRadius", sphereRadius);
        _fogSphereShader.setFloat("noiseInfluence", noiseInfluence);
        _fogSphereShader.setFloat("densityScale", fogDensity);
        _fogSphereShader.setFloat("transmittance", transmittance);
        _fogSphereShader.setVec3("fogColor", fogColor);
        _fogSphereShader.setVec3("lightAbsorb", lightAbsorbtion);

        // lights
        {
            _fogSphereShader.setInt("numDirectionalLightsBound",
                                    LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()
                                        ->getNumberOfBoundLights());
            _fogSphereShader.setInt("numPointLightsBound",
                                    LightManager<ComponentType::LIGHT_POINT>::instance()
                                        ->getNumberOfBoundLights());
            _fogSphereShader.setInt("numSpotLightsBound",
                                    LightManager<ComponentType::LIGHT_SPOT>::instance()
                                        ->getNumberOfBoundLights());
        }

        const auto fogBindingUnit = TextureManager::instance()->bindTexture(_fogTexture,
                                                                            GL_TEXTURE_3D);
        assert(fogBindingUnit != -1);
        _fogSphereShader.setInt("fogTexture", fogBindingUnit);

        {
            {
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                _currentPair = !_currentPair;

                {
                    const auto colorImageIdentifier = getCurrentWriteTarget().colorTexture;
                    const int colorImageHandle = *TextureManager::instance()->getTexture(
                        colorImageIdentifier);
                    glClearTexImage(colorImageHandle, 0, GL_RGBA, GL_FLOAT, clearColor);
                    // TODO: create a manager for this shit
                    glBindImageTexture(0, colorImageHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY,
                                       GL_RGBA16F);
                }

                {
                    const auto positionImageIdentifier = getCurrentWriteTarget().positionTexture;
                    const int positionImageHandle = *TextureManager::instance()->getTexture(
                        positionImageIdentifier);

                    glClearTexImage(positionImageHandle, 0, GL_RED, GL_FLOAT, clearPosition);
                    // TODO: create a manager for this shit
                    glBindImageTexture(1, positionImageHandle, 0, GL_FALSE, 0, GL_WRITE_ONLY,
                                       GL_R32F);
                }
            }

            _fogSphereShader
                .setGlobalDispatchDimenisons(
                    glm::uvec3(screenDiscretizationResoutionX / groupSize.x,
                               screenDiscretizationResoutionY / groupSize.y, 1))
                ->runShader();
        }

        MeshManager::instance()->unbindMesh();
        // TextureManager::instance()->unbindAllTextures();
    }
}

void VolumetricFogPass::syncTextureAccess(uint32_t syncBits) const
{
    _fogSphereShader.synchronizeGpuAccess(syncBits);
}

// public; returns the one for reading
TextureIdentifier VolumetricFogPass::colorTextureId() const
{
    return getCurrentReadTarget().colorTexture;
}

// public;  returns the one for reading
TextureIdentifier VolumetricFogPass::positionTextureId() const
{
    return getCurrentReadTarget().positionTexture;
}

bool VolumetricFogPass::volumeIsOutOfSight() const { return _volumeIsOutOfSight; }

void VolumetricFogPass::runShader()
{
    if (volumeIsOutOfSight())
        return;

    use();

    // TODO: consider using a different blending setup
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    syncTextureAccess(GL_TEXTURE_FETCH_BARRIER_BIT);
    const auto currentColorTextureId = colorTextureId();
    const auto currentPositionTextureId = positionTextureId();

    const auto colorBinding = TextureManager::instance()->bindTexture(currentColorTextureId);
    const auto depthBinding = TextureManager::instance()->bindTexture(currentPositionTextureId);

    assert(colorBinding != -1 && depthBinding != -1);

    const auto &[viewportOffsetX, viewportOffsetY, viewportSizeX,
                 viewportSizeY] = FrameBufferManager::instance()->getViewportDims();

    setInt("texColor", colorBinding);
    setInt("texDepth", depthBinding);
    setInt("viewportWidth", viewportSizeX);
    setInt("viewportHeight", viewportSizeY);

    MeshManager::instance()->bindMesh(MeshManager::instance()->getDummyMesh());
    glDrawArrays(GL_TRIANGLES, 0, 3);

    TextureManager::instance()->unbindTexture(currentColorTextureId);
    TextureManager::instance()->unbindTexture(currentPositionTextureId);
    MeshManager::instance()->unbindMesh();
}

void VolumetricFogPass::compileAndAttachNecessaryShaders(uint32_t id)
{
    if (_vertexShaderId == 0)
    {
        const std::string &vShaderCode = readShaderSource(_vertexPath);

        const char *vPtr = vShaderCode.c_str();

        _vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(_vertexShaderId, 1, &vPtr, 0);
        compileShader(_vertexShaderId);
    }

    glAttachShader(id, _vertexShaderId);

    if (_fragmentShaderId == 0)
    {
        const std::string &fShaderCode = readShaderSource(_fragmentPath);

        const char *fPtr = fShaderCode.c_str();

        _fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(_fragmentShaderId, 1, &fPtr, 0);
        compileShader(_fragmentShaderId);
    }

    glAttachShader(id, _fragmentShaderId);
}

void VolumetricFogPass::deleteShaders()
{
    glDeleteShader(_vertexShaderId);
    _vertexShaderId = 0;

    glDeleteShader(_fragmentShaderId);
    _fragmentShaderId = 0;
}

const VolumetricFogPass::RenderTargetPair &VolumetricFogPass::getCurrentReadTarget() const
{
    return _currentPair ? _readPair : _writePair;
}

const VolumetricFogPass::RenderTargetPair &VolumetricFogPass::getCurrentWriteTarget() const
{
    return _currentPair ? _writePair : _readPair;
}
