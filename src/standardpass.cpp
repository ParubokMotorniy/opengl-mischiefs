#include "standardpass.h"

#include "glad/glad.h"
#include "glm/glm.hpp"

#include "lightvisualizationshader.h"
#include "instancedshader.h"
#include "skyboxshader.h"
#include "worldplaneshader.h"


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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto [windowWidth, windowHeight] = _currentTargetWindow->currentWindowDimensions();

    const glm::mat4 projection = glm::perspective(glm::radians(_currentViewCamera->zoom()),
                                                  (float)windowWidth / windowHeight, 0.1f, 1000.0f);
    const glm::mat4 view = _currentViewCamera->getViewMatrix();

    {
        _shaderProgramMain->use();
        _shaderProgramMain->setMatrix4("view", view);
        _shaderProgramMain->setMatrix4("projection", projection);
        _shaderProgramMain->setVec3("viewPos", _currentViewCamera->position());
        _shaderProgramMain->runShader();
    }

    {
        _worldPlaneShader->use();

        // if (renderOnlyGrid)
        //     glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        _worldPlaneShader->setMatrix4("view", view);
        _worldPlaneShader->setMatrix4("projection", projection);

        _worldPlaneShader->runShader();

        // if (renderOnlyGrid)
        //     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
}

void StandardPass::setCamera(const Camera *newCamera) { _currentViewCamera = newCamera; }

void StandardPass::setWindow(const Window *newWindow) { _currentTargetWindow = newWindow; }
