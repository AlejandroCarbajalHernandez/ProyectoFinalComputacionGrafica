#ifndef TEPOZTLAN_SCENE_H
#define TEPOZTLAN_SCENE_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Model.h"
#include "Shader.h"

class TepoztlanScene {
public:
    glm::mat4 worldMatrix;

    TepoztlanScene();
    ~TepoztlanScene();

    void Init();
    void Draw(Shader& shader, GLint modelLoc);

private:
    Model* exconvento;
    Model* casas;
    Model* estacion;
    Model* kiosco;
    Model* letrero;
    Model* mercado;
    Model* meta;
};

#endif#pragma once
