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
    Model* building1Model;
    Model* building2Model;

    // Generador del suelo de montaña
    GLuint terrainVAO, terrainVBO;
    // Pista que sigue el trazo de Ruta.h
    GLuint roadVAO, roadVBO;
    int terrainCount;
    int roadStart[3];   // 0 asfalto, 1 raya central, 2 bordillos
    int roadCount[3];
    void BuildRoad();
    void DrawUrbana(Shader& shader, GLint modelLoc);   // E1 Polanco (0 a 210 m)
    void DrawRural(Shader& shader, GLint modelLoc);    // E2 ascenso rural (210 a 475 m)
    void DrawPueblo(Shader& shader, GLint modelLoc);   // E3 Tepoztlan (475 a 685 m)

    const int TOTAL_SEGMENTS = 25;
    const float SEGMENT_LENGTH = 3.0f; // Ajustado para eliminar fisuras entre tramos

    CicloviaScene();
    ~CicloviaScene();

    void Init();
    void Draw(Shader& shader, GLint modelLoc);
};