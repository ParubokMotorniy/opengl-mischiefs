#include "volumetricfogcomputepass.h"
#include "camera.h"
#include "texturemanager.h"

#include "glad/glad.h"
#include "imgui/imgui.h"

namespace
{
float fogDensity = 1.0f;
float sphereRadius = 10.0f;
constexpr glm::uvec3 groupSize = glm::uvec3(16, 16, 0);
constexpr size_t screenDiscretizationResoutionX = 1920;
constexpr size_t screenDiscretizationResoutionY = 1080;

constexpr float clearPosition[4] = { 1000.0f, 1000.0f, -1000.0f, 0.0f };
constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

} // namespace

VolumetricFogPass::VolumetricFogPass()
{
    _fogSphereShader.initializeShaderProgram();

    const auto renderImageCreator = [](int bindingPoint, TextureIdentifier &storeTarget) {
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

        // TODO: create a manager for this shit
        glBindImageTexture(bindingPoint, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

        storeTarget = TextureManager::instance()->registerTexture(texture);
    };

    renderImageCreator(0, _colorTexture);
    renderImageCreator(1, _positionTexture);
}

void VolumetricFogPass::runPass()
{
    {
        {
            const auto [viewportX, viewportY] = _currentWindow->currentWindowDimensions();
            ImGui::SetNextWindowPos(ImVec2(viewportX * 0.05, viewportY * 0.35), ImGuiCond_Always,
                                    ImVec2(0.0f, 0.5f));
        }
        ImGui::Begin("Fog parameters", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Fog sphere");
        ImGui::Separator();
        ImGui::SliderFloat("Fog density", &fogDensity, 0.001f, 10.0f);
        ImGui::SliderFloat("Fog sphere radius", &sphereRadius, 1.0f, 50.0f);
        ImGui::End();
    }

    {
        glClearTexImage(*TextureManager::instance()->getTexture(_positionTexture), 0, GL_RGBA,
                        GL_FLOAT, clearPosition);

        glClearTexImage(*TextureManager::instance()->getTexture(_colorTexture), 0, GL_RGBA,
                        GL_FLOAT, clearColor);
    }

    {
        _fogSphereShader.use();
        _fogSphereShader.setInt("resolutionX", screenDiscretizationResoutionX);
        _fogSphereShader.setInt("resolutionY", screenDiscretizationResoutionY);

        _fogSphereShader.setMatrix4("viewMatrix", _currentCamera->getViewMatrix());
        _fogSphereShader.setMatrix4("clipToView", glm::inverse(_currentCamera->projectionMatrix()));

        _fogSphereShader.setVec3("spherePos", glm::vec3(5.0f, 5.0f, 5.0f));
        _fogSphereShader.setFloat("sphereRadius", sphereRadius);
        _fogSphereShader.setFloat("densityScale", fogDensity);

        _fogSphereShader
            .setGlobalDispatchDimenisons(glm::uvec3(screenDiscretizationResoutionX / groupSize.x,
                                                    screenDiscretizationResoutionY / groupSize.y,
                                                    1))
            ->runShader();

        _fogSphereShader.synchronizeGpuAccess(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        MeshManager::instance()->unbindMesh();
        TextureManager::instance()->unbindAllTextures();
    }
}

TextureIdentifier VolumetricFogPass::colorTextureId() const { return _colorTexture; }

TextureIdentifier VolumetricFogPass::positionTextureId() const { return _positionTexture; }
