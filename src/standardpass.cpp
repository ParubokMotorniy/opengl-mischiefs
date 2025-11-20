#include "standardpass.h"

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "imgui.h"

#include "instancedshader.h"
#include "lightmanager.h"
#include "lightvisualizationshader.h"
#include "skyboxshader.h"
#include "texturemanager.h"
#include "worldplaneshader.h"
#include "camera.h"
#include "window.h"

namespace
{
void bindDirectionalShadowMaps(ShaderProgram *target)
{
    size_t mapCounter = 0;
    target->use();
    for (const DirectionalLight &l :
         LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()->getLights())
    {
        const int bindingPoint = TextureManager::instance()->bindTexture(l.shadowMapIdentifier);
        assert(bindingPoint != -1);
        const auto targetUniform = "directionalShadowMaps[" + std::to_string(mapCounter++) + "]";
        target->setInt(targetUniform, bindingPoint);
    }
}
} // namespace

StandardPass::StandardPass(InstancedShader *ins, WorldPlaneShader *wrld,
                           LightVisualizationShader *lightVis, SkyboxShader *skybox)
    : _shaderProgramMain(ins),
      _worldPlaneShader(wrld),
      _lightVisualizationShader(lightVis),
      _mainSkybox(skybox)
{
}

void StandardPass::runPass()
{
    _currentWindow->resetViewport();

    const glm::mat4 projection = _currentCamera->projectionMatrix();
    const glm::mat4 view = _currentCamera->getViewMatrix();

    static float directionalShadowBias = 0.0f;
    {
        ImGui::Begin("Standard pass parameters", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Shadow mapping");
        ImGui::Separator();
        ImGui::SliderFloat("Directional shadow bias", &directionalShadowBias, 0.001f, 0.05f);
        ImGui::End();
    }

    {
        _shaderProgramMain->use();
        bindDirectionalShadowMaps(_shaderProgramMain);
        _shaderProgramMain->setMatrix4("view", view);
        _shaderProgramMain->setMatrix4("projection", projection);
        _shaderProgramMain->setVec3("viewPos", _currentCamera->position());
        _shaderProgramMain->setFloat("directionalShadowBias", directionalShadowBias);
        _shaderProgramMain->setInt("numDirectionalLightsBound",
                                   LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()
                                       ->getNumberOfBoundLights());
        _shaderProgramMain->setInt("numPointLightsBound",
                                   LightManager<ComponentType::LIGHT_POINT>::instance()
                                       ->getNumberOfBoundLights());
        _shaderProgramMain->runShader();
        _shaderProgramMain
            ->setInt("numSpotLightsBound",
                     LightManager<ComponentType::LIGHT_SPOT>::instance()->getNumberOfBoundLights());
        _shaderProgramMain->setInt("numTexturedLightsBound",
                                   LightManager<ComponentType::LIGHT_TEXTURED_SPOT>::instance()
                                       ->getNumberOfBoundLights());
        TextureManager::instance()->unbindAllTextures();
        MeshManager::instance()->unbindMesh();
    }

    {
        _worldPlaneShader->use();
        bindDirectionalShadowMaps(_worldPlaneShader);
        _worldPlaneShader->setMatrix4("view", view);
        _worldPlaneShader->setMatrix4("projection", projection);
        _worldPlaneShader->setVec3("viewPos", _currentCamera->position());
        _worldPlaneShader->setFloat("directionalShadowBias", directionalShadowBias);
        _worldPlaneShader->setInt("numDirectionalLightsBound",
                                  LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()
                                      ->getNumberOfBoundLights());
        _worldPlaneShader->runShader();
        TextureManager::instance()->unbindAllTextures();
        MeshManager::instance()->unbindMesh();
    }
    {
        _lightVisualizationShader->use();
        _lightVisualizationShader->setMatrix4("view", view);
        _lightVisualizationShader->setMatrix4("projection", projection);
        _lightVisualizationShader->runShader();
        TextureManager::instance()->unbindAllTextures();
        MeshManager::instance()->unbindMesh();
    }

    {
        glDepthFunc(GL_LEQUAL);
        _mainSkybox->use();
        _mainSkybox->setMatrix4("view", glm::mat4(glm::mat3(view)));
        _mainSkybox->setMatrix4("projection", projection);
        _mainSkybox->runShader();
        glDepthFunc(GL_LESS);
        TextureManager::instance()->unbindAllTextures();
        MeshManager::instance()->unbindMesh();
    }
}
