#include "volumetricfogcomputepass.h"
#include "camera.h"
#include "texturemanager.h"

#include "glad/glad.h"
#include "imgui/imgui.h"

namespace
{
float fogDensity = 1.0f;
float sphereRadius = 10.0f;
} // namespace

VolumetricFogPass::VolumetricFogPass()
{
    _fogSphereShader.initializeShaderProgram();

    {
        unsigned int texture;

        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);

        // TODO: create a manager for this shit
        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

        _colorTexture = TextureManager::instance()->registerTexture(texture);
    }

    {
        unsigned int texture;

        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);

        // TODO: create a manager for this shit
        glBindImageTexture(1, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

        _positionTexture = TextureManager::instance()->registerTexture(texture);
    }
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
        _fogSphereShader.use();
        // TODO: extract exact values from current camera somehow
        // TODO: consider using a different blending setup
        _fogSphereShader.setInt("resolutionX", 1920);
        _fogSphereShader.setInt("resolutionY", 1080);
        _fogSphereShader.setVec3("spherePos", glm::vec3(5.0f, 5.0f, 5.0f));
        _fogSphereShader.setMatrix4("viewMatrix", _currentCamera->getViewMatrix());
        _fogSphereShader.setMatrix4("ndcToView", glm::inverse(_currentCamera->projectionMatrix()));
        _fogSphereShader.setFloat("densityScale", fogDensity);
        _fogSphereShader.setFloat("sphereRadius", sphereRadius);

        _fogSphereShader.setGlobalDispatchDimenisons(glm::uvec3(1920 / 16, 1080 / 16, 1))
            ->runShader();

        _fogSphereShader.synchronizeGpuAccess(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
}

TextureIdentifier VolumetricFogPass::colorTextureId() const { return _colorTexture; }

TextureIdentifier VolumetricFogPass::positionTextureId() const { return _positionTexture; }
