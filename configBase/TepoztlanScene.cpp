#include "TepoztlanScene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

TepoztlanScene::TepoztlanScene() {
    exconvento = nullptr;
    casas = nullptr;
    estacion = nullptr;
    kiosco = nullptr;
    letrero = nullptr;
    mercado = nullptr;
    worldMatrix = glm::mat4(1.0f);
}

TepoztlanScene::~TepoztlanScene() {
    delete exconvento;
    delete casas;
    delete estacion;
    delete kiosco;
    delete letrero;
    delete mercado;
}

void TepoztlanScene::Init() {
    exconvento = new Model((char*)"../assets/obj/ExconventoTepoztlan/ExconventoTepoztlan.obj");
    casas = new Model((char*)"../assets/obj/Casas/Casas.obj");
    estacion = new Model((char*)"../assets/obj/EstacionFerrocarril/EstacionFerrocarril.obj");
    kiosco = new Model((char*)"../assets/obj/Kiosco/Kiosco.obj");
    letrero = new Model((char*)"../assets/obj/LetreroTepoztlan/LetreroTepoztlan.obj");
    mercado = new Model((char*)"../assets/obj/MercadoMunicipal/MercadoMunicipal.obj");
}

void TepoztlanScene::Draw(Shader& shader, GLint modelLoc) {
    glm::mat4 model(1.0f);

    auto Base = [&](glm::vec3 offset, float rotacionY) {
        glm::mat4 m = worldMatrix;
        m = glm::translate(m, offset);
        m = glm::rotate(m, glm::radians(rotacionY), glm::vec3(0.0f, 1.0f, 0.0f));
        return m;
        };
    
    if (letrero) {
        model = Base(glm::vec3(13.5f, -0.5f, 5.0f), -18.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        letrero->Draw(shader);
    }

    if (estacion) {
        model = Base(glm::vec3(-18.0f, 0.3f, -15.0f), 180.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        estacion->Draw(shader);
    }

    if (kiosco) {
        model = Base(glm::vec3(20.0f, 0.3f, -35.0f), 0.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        kiosco->Draw(shader);
    }
    if (mercado) {
        model = Base(glm::vec3(40.0f, 0.3f, -40.0f), -90.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        mercado->Draw(shader);
    }

    if (exconvento) {
        model = Base(glm::vec3(5.5f, 0.4f, -53.0f), 0.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        exconvento->Draw(shader);
    }

    if (casas) {
        model = Base(glm::vec3(-30.0f, 0.3f, -50.0f), 0.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        casas->Draw(shader);
    }
}