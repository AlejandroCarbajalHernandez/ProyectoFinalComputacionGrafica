#pragma once

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Model.h"

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
    Model* camellon;     // Declarado como puntero
    Model* autoRojo;
    Model* autoNegro;
    Model* autobus;

    Model* edificioA;
    Model* edificioB;
    Model* edificioBeige; // Declarado como puntero
    Model* edificioC;
    Model* techoVerde;
    Model* torreCil;
    Model* torreBal;
    Model* torreCrisA;
    Model* torreCrisB;
    Model* carretera;

    glm::mat4 worldMatrix = glm::mat4(1.0f);

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
};