#include "gizmospass.h"

#include "camera.h"
#include "geometryshaderprogram.h"
#include "glad/glad.h"
#include "imgui.h"
#include "shadermanager.h"

namespace
{
bool renderAxes = false;
}

void GizmosPass::runPass()
{
    {
        {
            const auto [viewportX, viewportY] = _currentWindow->currentWindowDimensions();
            ImGui::SetNextWindowPos(ImVec2(viewportX * 0.05, viewportY * 0.2), ImGuiCond_Always,
                                    ImVec2(0.0f, 0.5f));
        }
        ImGui::Begin("Gizmos", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Transform");
        ImGui::Separator();
        ImGui::Checkbox("Toggle axes", &renderAxes);
        ImGui::End();
    }
    if (renderAxes)
    {
        ShaderProgram *_axesShader = ShaderManager::instance()->getShader("world_axes");

        glDisable(GL_DEPTH_TEST);
        _axesShader->use();

        _axesShader->setMatrix4("viewMat", _currentCamera->getViewMatrix());
        _axesShader->setMatrix4("projectionMat", _currentCamera->projectionMatrix());
        _axesShader->runShader();

        glEnable(GL_DEPTH_TEST);
    }
}
