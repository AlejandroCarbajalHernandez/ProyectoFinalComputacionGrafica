// TepoztlanScene.h: la escena del pueblo de Tepoztlán, el final del recorrido.
// Aquí se declara lo que se carga y se dibuja en esa zona; el detalle está en TepoztlanScene.cpp.
#ifndef TEPOZTLAN_SCENE_H
#define TEPOZTLAN_SCENE_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Model.h"
#include "Shader.h"

// Reúne los edificios del pueblo y la meta, y sabe dibujarlos.
class TepoztlanScene {
public:
    // Posición y orientación general de toda la escena en el mundo.
    glm::mat4 worldMatrix;

    // Crea la escena y libera sus modelos de la memoria al terminar.
    TepoztlanScene();
    ~TepoztlanScene();

    // Carga todos los modelos y deja la escena lista.
    void Init();
    // Dibuja la escena completa.
    void Draw(Shader& shader, GLint modelLoc);

private:
    // Los modelos 3D del pueblo: ex convento, casas, estación, kiosco, letrero, mercado y la meta.
    Model* exconvento;
    Model* casas;
    Model* estacion;
    Model* kiosco;
    Model* letrero;
    Model* mercado;
    Model* meta;
};

#endif#pragma once