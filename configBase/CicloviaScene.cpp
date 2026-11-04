#include "CicloviaScene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

CicloviaScene::CicloviaScene() {
    trackModel = nullptr;
    bridgeModel = nullptr;
    treeModel = nullptr;
    bushModel = nullptr;
    signModel = nullptr;
    trafficConeModel = nullptr;
    terrainVAO = 0;
    terrainVBO = 0;
}

CicloviaScene::~CicloviaScene() {
    delete trackModel;
    delete bridgeModel;
    delete treeModel;
    delete bushModel;
    delete signModel;
    delete trafficConeModel;
    if (terrainVAO) glDeleteVertexArrays(1, &terrainVAO);
    if (terrainVBO) glDeleteBuffers(1, &terrainVBO);
}

void CicloviaScene::Init() {
    trackModel = new Model((char*)"../assets/obj/pista/pista.obj");
    bridgeModel = new Model((char*)"../assets/obj/puente/puente.obj");
    treeModel = nullptr; // Asignar cuando esté listo arbol.obj
    bushModel = new Model((char*)"../assets/obj/arbusto/arbusto.obj");
    trafficConeModel = new Model((char*)"../assets/obj/tope/tope.obj");
    signModel = new Model((char*)"../assets/obj/senal-alto/senal-alto.obj");

    // Plano de terreno base (Pasto seco / tierra montañosa del Ajusco)
    float groundVertices[] = {
        // Coordenadas            // Normales         // TexCoords
        -60.0f, -0.05f, -120.0f,  0.0f, 1.0f, 0.0f,   0.0f, 50.0f,
         60.0f, -0.05f, -120.0f,  0.0f, 1.0f, 0.0f,  50.0f, 50.0f,
         60.0f, -0.05f,   30.0f,  0.0f, 1.0f, 0.0f,  50.0f,  0.0f,

         60.0f, -0.05f,   30.0f,  0.0f, 1.0f, 0.0f,  50.0f,  0.0f,
        -60.0f, -0.05f,   30.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -60.0f, -0.05f, -120.0f,  0.0f, 1.0f, 0.0f,   0.0f, 50.0f
    };

    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glBindVertexArray(terrainVAO);

    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void CicloviaScene::Draw(Shader& shader, GLint modelLoc) {
    glm::mat4 model(1.0f);

    // 1. Dibujar el plano de tierra/pasto seco que sostiene la ciclovía
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(terrainVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // 2. Tramos continuos de la pista de concreto
    for (int i = 0; i < TOTAL_SEGMENTS; i++) {
        float zOffset = -i * SEGMENT_LENGTH;

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, zOffset));
        // Ajuste longitudinal y escala de ancho para ciclovía de 2 carriles
        model = glm::scale(model, glm::vec3(0.25f, 0.15f, 0.25f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        if (trackModel) trackModel->Draw(shader);
    }

    // 3. Puente peatonal
    if (bridgeModel) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -48.0f));
        model = glm::scale(model, glm::vec3(0.22f, 0.22f, 0.22f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        bridgeModel->Draw(shader);
    }

    // 4. Confinadores / Topes
    if (trafficConeModel) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.3f, 0.0f, -14.0f));
        model = glm::scale(model, glm::vec3(0.12f, 0.12f, 0.12f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        trafficConeModel->Draw(shader);
    }

    // 5. Señalamiento de cicloturismo
    if (signModel) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.8f, 0.0f, -6.0f));
        model = glm::scale(model, glm::vec3(0.14f, 0.14f, 0.14f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        signModel->Draw(shader);
    }

    // 6. Vegetación a orillas de la terracería
    if (bushModel) {
        for (int i = 1; i < TOTAL_SEGMENTS; i += 3) {
            float zOffset = -i * SEGMENT_LENGTH;

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(3.2f, 0.0f, zOffset));
            model = glm::scale(model, glm::vec3(0.12f, 0.12f, 0.12f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            bushModel->Draw(shader);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-3.2f, 0.0f, zOffset - 2.0f));
            model = glm::scale(model, glm::vec3(0.12f, 0.12f, 0.12f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            bushModel->Draw(shader);
        }
    }
}