#include "transparentpass.h"

#include "shadermanager.h"
#include "transparentshader.h"

#include "glad/glad.h"

void SortingTransparentPass::runPass()
{
    glEnable(GL_BLEND);

    ShaderProgram *_transparentShader = ShaderManager::instance()->getShader("simple_transparent");
    _transparentShader->runShader();

    {
        ShaderProgram *_fogShader = ShaderManager::instance()->getShader("fullscreen_fog");
        _fogShader->use();
        _fogShader->runShader();
    }

    glDisable(GL_BLEND);
}

void SortingTransparentPass::setCamera(const Camera *currentCamera)
{
    _currentCamera = currentCamera;
    dynamic_cast<TransparentShader *>(ShaderManager::instance()->getShader("simple_transparent"))
        ->setCamera(currentCamera);
}
