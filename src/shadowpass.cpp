#include "shadowpass.h"

#include "framebuffermanager.h"
#include "instancedshader.h"
#include "lightmanager.h"
#include "lightvisualizationshader.h"

ShadowPass::ShadowPass(InstancedShader *ins, LightVisualizationShader *lightVis)
    : _shaderProgramMain(ins),
      _lightVisualizationShader(lightVis),
      _passThroughOverride(ENGINE_SHADERS "/depth_pass_through.vs",
                           ENGINE_SHADERS "/depth_pass_through.fs")
{
    _passThroughOverride.initializeShaderProgram();
}

void ShadowPass::runPass()
{
    for (const DirectionalLight &l :
         LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()->getLights())
    {
        FrameBufferManager::instance()->bindFrameBuffer(GL_FRAMEBUFFER, l.frameBufferId, 2048,
                                                        2048);
        glClear(GL_DEPTH_BUFFER_BIT);

        const glm::mat4 &projection = l.dummyProjectionMatrix;
        const glm::mat4 &view = l.dummyViewMatrix;
        const glm::vec3 &pos = l.dummyPosition;

        _shaderProgramMain->setShaderProgramOverride(_passThroughOverride);
        _lightVisualizationShader->setShaderProgramOverride(_passThroughOverride);

        {
            _shaderProgramMain->use();
            _shaderProgramMain->setMatrix4("view", view);
            _shaderProgramMain->setMatrix4("projection", projection);
            _shaderProgramMain->setVec3("viewPos", pos);
            _shaderProgramMain->runShader();
        }

        {
            _lightVisualizationShader->use();
            _lightVisualizationShader->setMatrix4("view", view);
            _lightVisualizationShader->setMatrix4("projection", projection);
            _lightVisualizationShader->runShader();
        }

        _shaderProgramMain->removeShaderProgramOverride();
        _lightVisualizationShader->removeShaderProgramOverride();

        // FrameBufferManager::instance()->unbindFrameBuffer(GL_FRAMEBUFFER);
        // TODO: reset the viewport after the pass so that the following passes don't have to
    }
}