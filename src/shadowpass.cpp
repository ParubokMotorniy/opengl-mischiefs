#include "shadowpass.h"

#include "framebuffermanager.h"
#include "instancedshader.h"
#include "lightmanager.h"
#include "lightvisualizationshader.h"
#include "skyboxshader.h"
#include "worldplaneshader.h"

ShadowPass::ShadowPass(InstancedShader *ins, WorldPlaneShader *wrld,
                       LightVisualizationShader *lightVis, SkyboxShader *skybox)
    : _shaderProgramMain(ins),
      _worldPlaneShader(wrld),
      _lightVisualizationShader(lightVis),
      _mainSkybox(skybox)
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
        FrameBufferManager::instance()->bindFrameBuffer(GL_FRAMEBUFFER, l.frameBufferId, 1024,
                                                        1024);

        const glm::mat4 projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 1000.0f);
        const auto defPos = glm::vec3(10.0f, 10.0f, 10.0f);
        const glm::mat4 view = glm::lookAt(defPos, defPos + l.dummyDirection,
                                           glm::vec3(0.0f, 1.0f, 0.0f));

        {
            _shaderProgramMain->use();
            _shaderProgramMain->setMatrix4("view", view);
            _shaderProgramMain->setMatrix4("projection", projection);
            _shaderProgramMain->setVec3("viewPos", defPos);
            _shaderProgramMain->runShader();
        }

        {
            _worldPlaneShader->use();

            _worldPlaneShader->setMatrix4("view", view);
            _worldPlaneShader->setMatrix4("projection", projection);

            _worldPlaneShader->runShader();
        }
        {
            _lightVisualizationShader->use();
            _lightVisualizationShader->setMatrix4("view", view);
            _lightVisualizationShader->setMatrix4("projection", projection);
            _lightVisualizationShader->runShader();
        }

        {
            glDepthFunc(GL_LEQUAL);
            _mainSkybox->use();
            _mainSkybox->setMatrix4("view", glm::mat4(glm::mat3(view)));
            _mainSkybox->setMatrix4("projection", projection);
            _mainSkybox->runShader();
            glDepthFunc(GL_LESS);
        }
        FrameBufferManager::instance()->unbindFrameBuffer(GL_FRAMEBUFFER);
    }
}