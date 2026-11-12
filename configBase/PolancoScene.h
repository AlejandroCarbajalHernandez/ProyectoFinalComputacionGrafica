#pragma once

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Model.h"
#include "Semaforo.h"

class PolancoScene {
public:
    // Modelos individuales
    Model* museo;
    Model* banca;
    Model* cono;
    Model* puente;
    Model* vias;
    Model* arbol;
    Model* luminaria;
    Model* camellon;
    Model* autoRojo;
    Model* autoNegro;
    Model* autobus;

    Model* edificioA;
    Model* edificioB;
    Model* edificioBeige;
    Model* edificioC;
    Model* techoVerde;
    Model* torreCil;
    Model* torreBal;
    Model* torreCrisA;
    Model* torreCrisB;
    Model* carreteraCruce;
    Model* carretera;
    Model* edificio1;
    Model* edificio2;
    Model* casa1;
    Model* casa2;
    Semaforo* semaforo;

    glm::mat4 worldMatrix = glm::mat4(1.0f);

    // Avance de la bici sobre la ruta (m). main.cpp lo actualiza en cada cuadro
    static float riderS;

    PolancoScene();
    ~PolancoScene();

    void Init();
    void Draw(Shader& shader, GLint modelLoc);

    struct Viewpoint {
        const char* name;
        glm::vec3   pos;
        float       rotDeg;
    };
    static const int VIEWPOINT_COUNT = 2;
    static const Viewpoint& GetViewpoint(int index);

    glm::vec3 GetSpawnPosition() const { return GetViewpoint(0).pos; }

private:
    struct Vehiculo {
        Model* modelo;
        float u0;      // posición inicial
        float u;       // posición actual
        float dir;     // +1 o -1
        float vel;     // m/s
        float largo;   // largo (m)
    };
    std::vector<Vehiculo> vehiculos;
    float cruceT0 = -1.0f;
    float ultimoT = 0.0f;
    void ActualizarVehiculos(float dt, bool verdeParaCarros);
};