#pragma once

#include <vector>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Shader.h"
#include "Model.h"

// Escena de Polanco (vista tipo foto aerea de referencia):
//  - Avenida a la derecha con trafico y autobus turistico
//  - Corredor del tren (vias) entre la avenida y el museo
//  - Calle lateral a la izquierda
//  - Museo Soumaya al fondo a la izquierda, con edificio de azotea verde al frente
//  - Torres de cristal (TorreCris / TorreCrisB) detras y a la derecha
//  - Arboles, luminarias, bancas, conos y puente peatonal
class PolancoScene {
public:
    PolancoScene();
    ~PolancoScene();

    void Init();
    void Draw(Shader& shader, GLint modelLoc);

    // Posicion sugerida de la bici (orilla izquierda de la avenida, junto a la banqueta)
    glm::vec3 GetSpawnPosition() const { return glm::vec3(9.6f, 0.0f, 10.0f); }

    // Texturas opcionales (0 = sin textura). Asignalas despues de Init().
    GLuint groundTex = 0;     // terreno
    GLuint roadTex = 0;       // asfalto
    GLuint sidewalkTex = 0;   // banquetas
    GLuint railBedTex = 0;    // balasto / tierra seca de las vias
    GLuint lawnTex = 0;       // pasto de la azotea verde
    GLuint wallTex = 0;       // muros blancos

private:
    struct Surface {
        GLuint vao = 0;
        GLuint vbo = 0;
        GLsizei count = 0;
    };

    struct VehicleSpawn {
        int   kind;   // 0 = CarroA, 1 = CarroB, 2 = AutoBusT
        float x, z;
        float rotY;   // grados
    };

    Surface ground, road, leftRoad, sidewalk, railBed, lawnRoof, whiteWall;

    // Modelos (nullptr si el .obj no se encontro)
    Model* museo = nullptr;
    Model* banca = nullptr;
    Model* cono = nullptr;
    Model* puente = nullptr;
    Model* vias = nullptr;
    std::vector<Model*> trees;
    std::vector<Model*> lamps;
    std::vector<Model*> buildings;   // en el orden de PATH_BUILDINGS (ver .cpp)
    Model* cars[3] = { nullptr, nullptr, nullptr };

    std::vector<VehicleSpawn> vehicles;

    Model* LoadModel(const std::string& path);
    void BuildSurface(Surface& s, const std::vector<float>& data);
    void DrawSurface(const Surface& s, GLuint tex);
    void DrawModel(Model* m, Shader& shader, GLint modelLoc,
                   const glm::vec3& pos, float rotYDeg, float scale);

    static void PushQuad(std::vector<float>& v,
                         glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
                         glm::vec3 n, float uRep, float vRep);
};
