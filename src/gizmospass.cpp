#include "gizmospass.h"

#include "camera.h"
#include "geometryshaderprogram.h"
#include "glad/glad.h"
#include "imgui.h"

namespace
{
bool renderAxes = false;
}

GizmosPass::GizmosPass(GeometryShaderProgram *axesShader) : _axesShader(axesShader) {}

void GizmosPass::runPass()
{
    {
        ImGui::Begin("Gizmos", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Transform");
        ImGui::Separator();
        ImGui::Checkbox("Toggle axes", &renderAxes);
        ImGui::End();
    }
    if (renderAxes)
    {
        glDisable(GL_DEPTH_TEST);
        _axesShader->use();

        _axesShader->setMatrix4("viewMat", _currentCamera->getViewMatrix());
        _axesShader->setMatrix4("projectionMat", _currentCamera->projectionMatrix());
        _axesShader->runShader();

        glEnable(GL_DEPTH_TEST);
    }
}
