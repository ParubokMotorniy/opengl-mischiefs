#include "transparentpass.h"

#include "transparentshader.h"

#include "glad/glad.h"

SortingTransparentPass::SortingTransparentPass(TransparentShader *transparentShader)
    : _transparentShader(transparentShader)
{
}

void SortingTransparentPass::runPass()
{
    glEnable(GL_BLEND);

    _transparentShader->runShader();

    glDisable(GL_BLEND);
}

void SortingTransparentPass::setCamera(const Camera *currentCamera)
{
    _currentCamera = currentCamera;
    _transparentShader->setCamera(currentCamera);
}
