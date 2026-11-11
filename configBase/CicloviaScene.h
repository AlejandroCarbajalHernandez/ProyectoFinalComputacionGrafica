#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Model.h"
#include "PolancoScene.h"
#include "TepoztlanScene.h"
#include <vector>

class CicloviaScene {
public:
    // Modelos generales
    Model* trackModel;
    Model* bridgeModel;
    Model* treeModel;          // Ocote / Pino montañés
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

    // Aves y Señalización
    Model* aveModel;
    Model* semaforoModel;
    void DrawSemaforos(Shader& shader, GLint modelLoc);
    void DrawAves(Shader& shader, GLint modelLoc);

    // Fauna rural
    Model* perroModel;
    Model* cabraModel;
    Model* ovejaModel;
    Model* caballoModel;

    PolancoScene polanco;
    TepoztlanScene tepoztlan;

    // Terreno base (verde) y manto árido (tepetate)
    GLuint terrainVAO, terrainVBO;
    GLuint aridVAO, aridVBO;
    int terrainCount;
    int aridCount;

    // Pista: 0 Asfalto, 1 Raya amarilla, 2 Bordillos, 3 Terracería rústica
    GLuint roadVAO, roadVBO;
    int roadStart[4];
    int roadCount[4];

    void BuildRoad();
    void BuildAridTerrain();
    void DrawUrbana(Shader& shader, GLint modelLoc);
    void DrawRural(Shader& shader, GLint modelLoc);
    void DrawPueblo(Shader& shader, GLint modelLoc);

    // Renderizado jerárquico articular para cuadrúpedos
    void DrawCuadrupedo(Model* m, Shader& shader, GLint modelLoc,
        const glm::mat4& root, float walkPhase,
        float xHombro, float xCadera, float yCadera,
        float ampDeg = 20.0f);

    const int TOTAL_SEGMENTS = 25;
    const float SEGMENT_LENGTH = 3.0f;

    CicloviaScene();
    ~CicloviaScene();

    void Init();
    void Draw(Shader& shader, GLint modelLoc);
};