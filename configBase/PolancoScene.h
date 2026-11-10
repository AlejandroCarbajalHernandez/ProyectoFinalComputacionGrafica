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
    Model* torreCrisB;     // carretera paralela a la ciclovia
    Model* carreteraCruce;  
    Model* carretera;
    Model* edificio1;
    Model* edificio2;
    Model* casa2;
    Model*casa1;
    Semaforo* semaforo;   // <--- Debe ser de tipo Semaforo*, NO Model*
    // Puedes agregar más según los nombres de los archivos que tengas// NUEVO: carretera que cruza la ciclovia (con los autos)

     // semaforos del cruce y de la avenida

    glm::mat4 worldMatrix = glm::mat4(1.0f);

    // Avance de la bici sobre la ruta (m). main.cpp lo actualiza en cada cuadro
    // y el cruce lo usa para saber cuando activar la animacion.
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
    // Trafico del cruce
    struct Vehiculo {
        Model* modelo;
        float u0;      // posicion inicial a lo largo de la carretera
        float u;       // posicion actual (0 = eje de la ciclovia)
        float dir;     // +1 hacia +X, -1 hacia -X
        float vel;     // m/s
        float largo;   // largo del vehiculo (m)
    };
    std::vector<Vehiculo> vehiculos;
    float cruceT0 = -1.0f;       // instante en que se activo el cruce (-1 = aun no)
    float ultimoT = 0.0f;
    void ActualizarVehiculos(float dt, bool verdeParaCarros);
};