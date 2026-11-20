// CicloviaScene.h: la escena completa de la ciclovía, desde la ciudad hasta Tepoztlán.
// Aquí se declara todo lo que se carga y se dibuja del recorrido; el detalle está en CicloviaScene.cpp.
#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Model.h"
#include "PolancoScene.h"
#include "TepoztlanScene.h"
#include <vector>

// Reúne los modelos, el terreno y la carretera del recorrido, y sabe dibujar cada tramo.
class CicloviaScene {
public:
    // Elementos que se ven a lo largo de todo el recorrido: puentes, árboles, montañas, señales, edificios, etc.
    // Modelos generales
    Model* trackModel;
    Model* bridgeModel;
    Model* treeModel;          // Ocote / Pino montañés
    Model* ocoteModel;         // Ocote real con corteza
    Model* mountainModel;
    Model* bushModel;
    Model* signModel;
    Model* trafficConeModel;
    Model* building1Model;
    Model* building2Model;

    // Vegetación y objetos de la zona árida y del campo.
    // Bioma árido y rural
    Model* huizacheModel;
    Model* nopalModel;
    Model* magueyModel;
    Model* zacatonModel;
    Model* gavillaModel;
    Model* tepetateModel;
    Model* torreModel;

    // El ave y el semáforo, junto con las funciones que los dibujan.
    // Aves y Señalización
    Model* aveModel;
    Model* semaforoModel;
    void DrawSemaforos(Shader& shader, GLint modelLoc);
    void DrawAves(Shader& shader, GLint modelLoc);

    // Animales del campo.
    // Fauna rural
    Model* perroModel;
    Model* cabraModel;
    Model* ovejaModel;
    Model* caballoModel;

    // Las escenas de los dos extremos del recorrido: la zona de Polanco y el pueblo de Tepoztlán.
    PolancoScene polanco;
    TepoztlanScene tepoztlan;

    // Datos del suelo del escenario, ya preparados para dibujarse.
    // Terreno base y manto árido
    GLuint terrainVAO, terrainVBO;
    GLuint aridVAO, aridVBO;
    int terrainCount;
    int aridCount;

    // Datos de la carretera: cada tipo de superficie se guarda por separado.
    // Pista: 0 Asfalto, 1 Raya amarilla, 2 Bordillos, 3 Terracería rústica
    GLuint roadVAO, roadVBO;
    int roadStart[4];
    int roadCount[4];

    // Preparan la carretera y el terreno árido antes de dibujar.
    void BuildRoad();
    void BuildAridTerrain();
    // Dibujan cada tramo del recorrido: la ciudad, el campo y el pueblo.
    void DrawUrbana(Shader& shader, GLint modelLoc);
    void DrawRural(Shader& shader, GLint modelLoc);
    void DrawPueblo(Shader& shader, GLint modelLoc);

    // Dibuja un animal de cuatro patas y mueve sus patas como si caminara.
    // Los números indican dónde están sus hombros y caderas, y qué tanto se abren las patas al caminar.
    void DrawCuadrupedo(Model* m, Shader& shader, GLint modelLoc,
        const glm::mat4& root, float walkPhase,
        float xHombro, float xCadera, float yCadera,
        float ampDeg = 20.0f);

    // La carretera se arma con 25 tramos de 3 metros cada uno.
    const int TOTAL_SEGMENTS = 25;
    const float SEGMENT_LENGTH = 3.0f;

    // Crea la escena y libera sus modelos de la memoria al terminar.
    CicloviaScene();
    ~CicloviaScene();

    // Carga todos los modelos y deja la escena lista.
    void Init();
    // Dibuja la escena completa.
    void Draw(Shader& shader, GLint modelLoc);
};