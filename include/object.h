#pragma once

#include "mesh.h"
#include "material.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// one primitive (for now) object means:
//  + one mesh
//  + one material
//  + one texture

struct PrimitiveObject
{
    std::string objMesh;
    Material objMaterial;

    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::mat4 rotation = glm::identity<glm::mat4>();
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::mat4 computeModelMatrix() const
    {
        glm::mat4 modelM = glm::identity<glm::mat4>();
        modelM = glm::scale(modelM, scale);
        modelM = rotation * modelM;
        modelM = glm::translate(modelM, position);

        return modelM;
    }

    glm::mat4 computeModelMatrixNoScale() const 
    {
        glm::mat4 modelM = glm::identity<glm::mat4>();
        modelM = rotation * modelM;
        modelM = glm::translate(modelM, position);

        return modelM;
    }
};
