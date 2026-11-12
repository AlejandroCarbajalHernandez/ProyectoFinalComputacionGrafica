#include "TepoztlanScene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Constructor: inicializa punteros de modelos del pueblo en nulo y establece la matriz mundial base
TepoztlanScene::TepoztlanScene() {
    exconvento = nullptr;
    casas = nullptr;
    estacion = nullptr;
    kiosco = nullptr;
    letrero = nullptr;
    mercado = nullptr;
    meta = nullptr;
    worldMatrix = glm::mat4(1.0f);
}

// Destructor: libera la memoria dinámica asignada a cada modelo arquitectónico de Tepoztlán
TepoztlanScene::~TepoztlanScene() {
    delete exconvento;
    delete casas;
    delete estacion;
    delete kiosco;
    delete letrero;
    delete mercado;
    delete meta;
}

// Carga desde disco los archivos .obj correspondientes a la arquitectura y arco de meta del destino
void TepoztlanScene::Init() {
    exconvento = new Model((char*)"../assets/obj/ExconventoTepoztlan/ExconventoTepoztlan.obj");
    casas = new Model((char*)"../assets/obj/Casas/Casas.obj");
    estacion = new Model((char*)"../assets/obj/EstacionFerrocarril/EstacionFerrocarril.obj");
    kiosco = new Model((char*)"../assets/obj/Kiosco/Kiosco.obj");
    letrero = new Model((char*)"../assets/obj/LetreroTepoztlan/LetreroTepoztlan.obj");
    mercado = new Model((char*)"../assets/obj/MercadoMunicipal/MercadoMunicipal.obj");
    meta = new Model((char*)"../assets/obj/meta/meta.obj");
}

// Posiciona y renderiza todos los elementos de Tepoztlán relativos a worldMatrix (metro 475 de la ruta)
void TepoztlanScene::Draw(Shader& shader, GLint modelLoc) {
    glm::mat4 model(1.0f);

    // Función auxiliar que aplica traslación y rotación en Y respecto al origen global del pueblo
    auto Base = [&](glm::vec3 offset, float rotacionY) {
        glm::mat4 m = worldMatrix;
        m = glm::translate(m, offset);
        m = glm::rotate(m, glm::radians(rotacionY), glm::vec3(0.0f, 1.0f, 0.0f));
        return m;
        };

    // 1. Arco de meta en la entrada del pueblo
    if (meta) {
        model = Base(glm::vec3(0.0f, 0.02f, -6.0f), 0.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        meta->Draw(shader);
    }

    // 2. Letrero monumental de bienvenida a Tepoztlán
    if (letrero) {
        model = Base(glm::vec3(13.5f, -0.5f, 5.0f), -18.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        letrero->Draw(shader);
    }

    // 3. Antigua estación del ferrocarril (costado izquierdo)
    if (estacion) {
        model = Base(glm::vec3(-18.0f, 0.3f, -15.0f), 180.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        estacion->Draw(shader);
    }

    // 4. Kiosco central de la plaza
    if (kiosco) {
        model = Base(glm::vec3(20.0f, 0.3f, -35.0f), 0.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        kiosco->Draw(shader);
    }

    // 5. Mercado municipal
    if (mercado) {
        model = Base(glm::vec3(40.0f, 0.3f, -40.0f), -90.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        mercado->Draw(shader);
    }

    // 6. Exconvento de la Natividad al fondo del recorrido
    if (exconvento) {
        model = Base(glm::vec3(5.5f, 0.4f, -53.0f), 0.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        exconvento->Draw(shader);
    }

    // 7. Conjunto de casas tradicionales del pueblo
    if (casas) {
        model = Base(glm::vec3(-30.0f, 0.3f, -50.0f), 0.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        casas->Draw(shader);
    }
}