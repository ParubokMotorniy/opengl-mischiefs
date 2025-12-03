#include "transparentpass.h"

#include "fullscreenfogshader.h"
#include "transparentshader.h"

#include "glad/glad.h"

SortingTransparentPass::SortingTransparentPass(TransparentShader *transparentShader,
                                               FullscreenFogShader *fogShader)
    : _transparentShader(transparentShader), _fogShader(fogShader)
{
}

void SortingTransparentPass::runPass()
{
    glEnable(GL_BLEND);

    _transparentShader->runShader();

    {
        _fogShader->use();
        _fogShader->setMatrix4("projection", _currentCamera->projectionMatrix());
        _fogShader->runShader();
    }

    glDisable(GL_BLEND);
}

void SortingTransparentPass::setCamera(const Camera *currentCamera)
{
    _currentCamera = currentCamera;
    _transparentShader->setCamera(currentCamera);
}
