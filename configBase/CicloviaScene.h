#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Shader.h"
#include "Model.h"

class CicloviaScene {
public:
    Model* trackModel;
    Model* bridgeModel;
    Model* treeModel;
    Model* mountainModel;
    Model* bushModel;
    Model* signModel;
    Model* trafficConeModel;

    // Generador del suelo de montaña
    GLuint terrainVAO, terrainVBO;

    const int TOTAL_SEGMENTS = 25;
    const float SEGMENT_LENGTH = 3.0f; // Ajustado para eliminar fisuras entre tramos

    CicloviaScene();
    ~CicloviaScene();

    void Init();
    void Draw(Shader& shader, GLint modelLoc);
};