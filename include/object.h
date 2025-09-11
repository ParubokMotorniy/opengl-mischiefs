#pragma once

#include "mesh.h"
#include "material.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//one primitive (for now) object means: 
// + one mesh
// + one material
// + one texture

struct PrimitiveObject
{
    std::string objMesh;
    Material objMaterial;

    glm::vec3 scale;
    glm::mat4 rotation;
    glm::vec3 position; 
};
