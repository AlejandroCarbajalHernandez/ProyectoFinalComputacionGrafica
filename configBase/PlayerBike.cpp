#include "PlayerBike.h"
#include "RiderPose.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Ruta.h"

PlayerBike::PlayerBike() {
    // Apoyado directamente sobre la superficie de la pista
    position = glm::vec3(0.0f, 0.20f, 0.0f);
    rotation = 0.0f;
    speed = 6.0f;
    turnSpeed = 65.0f;
    wheelRotation = 0.0f;
    pitch = 0.0f;

    bikeModel = nullptr;
    bikerModel = nullptr;
    helmetModel = nullptr;
    backpackModel = nullptr;
}

PlayerBike::~PlayerBike() {
    delete bikeModel;
    delete bikerModel;
    delete helmetModel;
    delete backpackModel;
}

void PlayerBike::Init() {
    bikeModel = new Model((char*)"../assets/obj/bicicleta/bicicleta.obj");
    bikeModel->SetColorMatching("Cuadro", glm::vec3(0.80f, 0.10f, 0.10f));
    bikeModel->SetColorMatching("Caucho", glm::vec3(0.05f, 0.05f, 0.05f));
    bikeModel->SetColorMatching("Radios", glm::vec3(0.90f, 0.90f, 0.90f));
    bikeModel->SetColorMatching("Maza", glm::vec3(0.60f, 0.60f, 0.65f));
    bikeModel->SetColorMatching("Sillin", glm::vec3(0.10f, 0.10f, 0.10f));
    bikeModel->SetColorMatching("Manubrio", glm::vec3(0.10f, 0.10f, 0.10f));

    bikerModel = new Model((char*)"../assets/obj/ciclista1/modelado-ciclista-blender.obj");
    // Reparte los vertices del ciclista en segmentos (piernas, brazos, torso) para animarlo
    bikerModel->AssignBones(RiderPose::BoneForVertex);
    helmetModel = new Model((char*)"../assets/obj/casco-ciclista/casco-ciclista.obj");
    backpackModel = new Model((char*)"../assets/obj/equipaje/equipaje.obj");
}

void PlayerBike::ProcessInput(bool keys[], float deltaTime) {
    float velocity = speed * deltaTime;
    float angularVelocity = turnSpeed * deltaTime;

    // Avanzar (W o Flecha Arriba)
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) {
        position.x -= sin(glm::radians(rotation)) * velocity;
        position.z -= cos(glm::radians(rotation)) * velocity;
        wheelRotation += 350.0f * deltaTime; // Giro continuo de las llantas
    }
    // Frenar / Reversa
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) {
        position.x += sin(glm::radians(rotation)) * (velocity * 0.4f);
        position.z += cos(glm::radians(rotation)) * (velocity * 0.4f);
        wheelRotation -= 180.0f * deltaTime;
    }
    // Giro de manubrio y trayectoria
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) {
        rotation += angularVelocity;
    }
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) {
        rotation -= angularVelocity;
    }

    // Seguir el relieve: altura e inclinacion segun el suelo bajo la bici
    glm::vec3 fwd(-sin(glm::radians(rotation)), 0.0f, -cos(glm::radians(rotation)));
    float hF = Ruta::AlturaSuelo(position.x + fwd.x * 0.8f, position.z + fwd.z * 0.8f);
    float hB = Ruta::AlturaSuelo(position.x - fwd.x * 0.8f, position.z - fwd.z * 0.8f);
    position.y = 0.5f * (hF + hB) + 0.20f;
    pitch = glm::degrees(atan2(hF - hB, 1.6f));
}

void PlayerBike::Draw(Shader& shader, GLint modelLoc, bool firstPerson) {
    // 1. Matriz raíz: posición en el mundo y dirección de manejo
    glm::mat4 root(1.0f);
    root = glm::translate(root, position);
    root = glm::rotate(root, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    root = glm::rotate(root, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    root = glm::scale(root, glm::vec3(0.35f, 0.35f, 0.35f));

    // 2. Chasis de la bicicleta
    glm::mat4 modelBike = root;
    // Solo la bicicleta necesita corrección de 90° por cómo se exportó de Blender
    modelBike = glm::rotate(modelBike, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBike));
    if (bikeModel) {
        // Cuadro, manubrio y asiento (todo menos las ruedas)
        bikeModel->DrawExcluding(shader, "Trasera", "Delantera");

        // Ruedas: giran sobre su propio centro (eje Z de la bicicleta).
        // Centros medidos en el modelo: trasera x=-0.52, delantera x=0.55.
        // El signo menos hace que giren hacia adelante al avanzar.
        float centros[2] = { -0.52f, 0.55f };
        const char* nombres[2] = { "Trasera", "Delantera" };
        for (int w = 0; w < 2; w++) {
            glm::mat4 modelWheel = modelBike;
            modelWheel = glm::translate(modelWheel, glm::vec3(centros[w], 0.0f, 0.0f));
            modelWheel = glm::rotate(modelWheel, glm::radians(-wheelRotation), glm::vec3(0.0f, 0.0f, 1.0f));
            modelWheel = glm::translate(modelWheel, glm::vec3(-centros[w], 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelWheel));
            bikeModel->DrawMatching(shader, nombres[w]);
        }
    }

    // 3. Bolsa / Maleta (Se empuja hacia adelante en -Z y arriba en Y)
    glm::mat4 modelBag = root;
    modelBag = glm::translate(modelBag, glm::vec3(0.0f, 0.95f, -0.60f));
    modelBag = glm::scale(modelBag, glm::vec3(0.50f, 0.50f, 0.50f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBag));
    if (backpackModel) backpackModel->Draw(shader);

    // 4. Ciclista y Casco
    if (!firstPerson) {
        // Pose por segmentos: sentado en el sillín, manos en el manubrio y piernas pedaleando.
        // El ángulo de la biela avanza con el giro de las ruedas.
        glm::mat4 bones[RiderPose::BONE_COUNT];
        float pedalAngle = glm::radians(-wheelRotation * 0.35f);
        RiderPose::Compute(pedalAngle, bones);

        glm::mat4 modelBiker = root;
        modelBiker = glm::translate(modelBiker, RiderPose::RIDER_OFFSET);
        modelBiker = glm::rotate(modelBiker, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBiker));

        glUniform1i(glGetUniformLocation(shader.Program, "useBones"), 1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "bones"), RiderPose::BONE_COUNT, GL_FALSE, glm::value_ptr(bones[0]));
        if (bikerModel) bikerModel->Draw(shader);
        glUniform1i(glGetUniformLocation(shader.Program, "useBones"), 0);

        // Casco: sigue a la cabeza, que se levanta para mirar al frente
        glm::mat4 modelHelmet = modelBiker * bones[RiderPose::HEAD];
        modelHelmet = glm::translate(modelHelmet, glm::vec3(0.0f, 1.88f, 0.065f));
        modelHelmet = glm::rotate(modelHelmet, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelHelmet = glm::scale(modelHelmet, glm::vec3(0.115f, 0.21f, 0.105f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelHelmet));
        if (helmetModel) helmetModel->Draw(shader);
    }
}

glm::vec3 PlayerBike::GetFrontVector() {
    return glm::vec3(-sin(glm::radians(rotation)), 0.0f, -cos(glm::radians(rotation)));
}
