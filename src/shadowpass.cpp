#include "shadowpass.h"

#include "framebuffermanager.h"
#include "instancedshader.h"
#include "lightmanager.h"
#include "lightvisualizationshader.h"
#include "skyboxshader.h"
#include "worldplaneshader.h"

ShadowPass::ShadowPass(InstancedShader *ins, LightVisualizationShader *lightVis)
    : _shaderProgramMain(ins), _lightVisualizationShader(lightVis)
{
}

void ShadowPass::runPass()
{
    // TODO:
    // for each light,
    //  1. bind its frame buffer, along with texture
    //  2. uniformly bind the matrices
    //  3. run all shaders (TODO: THIS IS HORRIBLY HEAVY. THE SHADERS MUST BE SIMPLIFIED TO ONLY
    //  OUTPUT THE DEPTH)
    //  4. unbind the frame buffer

    for (const DirectionalLight &l :
         LightManager<ComponentType::LIGHT_DIRECTIONAL>::instance()->getLights())
    {
        FrameBufferManager::instance()->bindFrameBuffer(GL_FRAMEBUFFER, l.frameBufferId, 2048,
                                                        2048);

        const glm::mat4 &projection = l.dummyProjectionMatrix;
        const glm::mat4 &view = l.dummyViewMatrix;
        const glm::vec3 &pos = l.dummyPosition;

        glClear(GL_DEPTH_BUFFER_BIT);

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

        FrameBufferManager::instance()->unbindFrameBuffer(GL_FRAMEBUFFER);
    }
}