#include "volumetricfogcomputepass.h"
#include "camera.h"
#include "texturemanager.h"

#include "glad/glad.h"

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);

        // TODO: create a manager for this shit
        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1920, 1080, 0, GL_RGBA, GL_FLOAT, NULL);

        // TODO: create a manager for this shit
        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

        _positionTexture = TextureManager::instance()->registerTexture(texture);
    }
}

void VolumetricFogPass::runPass()
{
    {
        _fogSphereShader.use();
        // TODO: extract exact values from current camera somehow

        _fogSphereShader.setFloat("aspectRatio", 1920.0f / 1080.0f);
        _fogSphereShader.setFloat("nearClip", 0.1f);
        _fogSphereShader.setInt("resolutionX", 1920);
        _fogSphereShader.setInt("resolutionY", 1080);
        _fogSphereShader.setVec3("spherePos", glm::vec3(5.0f, 5.0f, 5.0f));
        _fogSphereShader.setFloat("sphereRadius", 10.0f);
        _fogSphereShader.setMatrix4("viewMatrix", _currentCamera->getViewMatrix());

        _fogSphereShader.setGlobalDispatchDimenisons(glm::uvec3(1920, 1080, 1))->runShader();

        _fogSphereShader.synchronizeGpuAccess(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
}

TextureIdentifier VolumetricFogPass::colorTextureId() const { return _colorTexture; }

TextureIdentifier VolumetricFogPass::positionTextureId() const { return _positionTexture; }
