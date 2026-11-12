// PolancoScene.h: la escena de Polanco, el inicio urbano del recorrido.
// Aquí se declara lo que se carga y se dibuja en esa zona; el detalle está en PolancoScene.cpp.
#pragma once

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Model.h"
#include "Semaforo.h"

// Reúne los edificios, el cruce con semáforo, los vehículos y los puntos de inicio de la zona de Polanco.
class PolancoScene {
public:
    // Los modelos 3D de la zona: museo, banca, conos, puente, vías, árboles, luminarias, camellón y vehículos.
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

    // Edificios, torres, casas y carreteras que forman la ciudad.
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
    // El semáforo del cruce.
    Semaforo* semaforo;

    // Posición y orientación general de toda la escena en el mundo.
    glm::mat4 worldMatrix = glm::mat4(1.0f);

    // Avance de la bici sobre la ruta (m). main.cpp lo actualiza en cada cuadro
    static float riderS;

    // Crea la escena y libera sus modelos de la memoria al terminar.
    PolancoScene();
    ~PolancoScene();

    // Carga todos los modelos y deja la escena lista.
    void Init();
    // Dibuja la escena completa.
    void Draw(Shader& shader, GLint modelLoc);

    // Un punto de vista de la escena: su nombre, dónde se coloca la cámara y hacia dónde mira.
    struct Viewpoint {
        const char* name;
        glm::vec3   pos;
        float       rotDeg;
    };
    // Cantidad de puntos de vista disponibles y función para obtener uno.
    static const int VIEWPOINT_COUNT = 2;
    static const Viewpoint& GetViewpoint(int index);

    // Dónde empieza el jugador: el primer punto de vista.
    glm::vec3 GetSpawnPosition() const { return GetViewpoint(0).pos; }

private:
    // Un vehículo que circula por el cruce: su modelo y cómo se mueve.
    struct Vehiculo {
        Model* modelo;
        float u0;      // posición inicial
        float u;       // posición actual
        float dir;     // +1 o -1
        float vel;     // m/s
        float largo;   // largo (m)
    };
    // Los vehículos de la escena y el control del tiempo del semáforo.
    std::vector<Vehiculo> vehiculos;
    float cruceT0 = -1.0f;
    float ultimoT = 0.0f;
    // Mueve los vehículos según el tiempo y hace que se detengan cuando no tienen luz verde.
    void ActualizarVehiculos(float dt, bool verdeParaCarros);
};