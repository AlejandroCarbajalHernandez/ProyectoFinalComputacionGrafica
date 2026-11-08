#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Model.h"
#include "PolancoScene.h"

class CicloviaScene {
public:
    // Modelos generales
    Model* trackModel;
    Model* bridgeModel;
    Model* treeModel;          // Ocote
    Model* mountainModel;
    Model* bushModel;
    Model* signModel;
    Model* trafficConeModel;
    Model* building1Model;
    Model* building2Model;

    // Bioma árido y rural
    Model* huizacheModel;
    Model* nopalModel;
    Model* magueyModel;
    Model* zacatonModel;
    Model* gavillaModel;
    Model* tepetateModel;
    Model* torreModel;

    // Fauna rural
    Model* perroModel;
    Model* cabraModel;
    Model* ovejaModel;
    Model* caballoModel;

    PolancoScene polanco;

    // Terreno y manto árido
    GLuint terrainVAO, terrainVBO;
    GLuint aridVAO, aridVBO;
    int terrainCount;
    int aridCount;

    // Pista: Asfalto, raya, bordillos y terracería separadas
    GLuint roadVAO, roadVBO;
    int roadStart[4]; // 0: Asfalto, 1: Raya amarilla, 2: Bordillo concreto, 3: Terracería rústica
    int roadCount[4];

    void BuildRoad();
    void BuildAridTerrain();
    void DrawUrbana(Shader& shader, GLint modelLoc);
    void DrawRural(Shader& shader, GLint modelLoc);
    void DrawPueblo(Shader& shader, GLint modelLoc);

    const int TOTAL_SEGMENTS = 25;
    const float SEGMENT_LENGTH = 3.0f;

    CicloviaScene();
    ~CicloviaScene();

    void Init();
    void Draw(Shader& shader, GLint modelLoc);
};