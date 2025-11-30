#include "shadowpass.h"

#include "framebuffermanager.h"
#include "instancedshader.h"
#include "lightmanager.h"
#include "lightvisualizationshader.h"
#include "pbrshader.h"

ShadowPass::ShadowPass(InstancedBlinnPhongShader *ins, LightVisualizationShader *lightVis,
                       PbrShader *pbrShader)
    : _shaderProgramMain(ins),
      _lightVisualizationShader(lightVis),
      _pbrShader(pbrShader),
      _passThroughOverride(ENGINE_SHADERS "/depth_pass_through.vs",
                           ENGINE_SHADERS "/depth_pass_through.fs"),
      _passThroughOverridePbr(ENGINE_SHADERS "/depth_pass_through_pbr.vs",
                           ENGINE_SHADERS "/depth_pass_through.fs")
{
    _passThroughOverride.initializeShaderProgram();
    _passThroughOverridePbr.initializeShaderProgram();
}

void ShadowPass::runPass()
{
    for (const DirectionalLight &l :
         LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()->getLights())
    {
        FrameBufferManager::instance()->bindFrameBuffer(GL_FRAMEBUFFER, l.frameBufferId, 2048,
                                                        2048);
        glViewport(0, 0, 2048, 2048);

        glClear(GL_DEPTH_BUFFER_BIT);

        const glm::mat4 &projection = l.dummyProjectionMatrix;
        const glm::mat4 &view = l.dummyViewMatrix;

        _shaderProgramMain->setShaderProgramOverride(_passThroughOverride);
        _lightVisualizationShader->setShaderProgramOverride(_passThroughOverride);
        _pbrShader->setShaderProgramOverride(_passThroughOverridePbr);

        {
            _shaderProgramMain->use();
            _shaderProgramMain->setMatrix4("view", view);
            _shaderProgramMain->setMatrix4("projection", projection);
            _shaderProgramMain->runShader();
        }

        {
            _lightVisualizationShader->use();
            _lightVisualizationShader->setMatrix4("view", view);
            _lightVisualizationShader->setMatrix4("projection", projection);
            _lightVisualizationShader->runShader();
        }

        {
            _pbrShader->use();
            _pbrShader->setMatrix4("view", view);
            _pbrShader->setMatrix4("projection", projection);
            _pbrShader->runShader();
        }

        _shaderProgramMain->removeShaderProgramOverride();
        _lightVisualizationShader->removeShaderProgramOverride();
        _pbrShader->removeShaderProgramOverride();
    }
    FrameBufferManager::instance()->unbindFrameBuffer(GL_FRAMEBUFFER);
}