#include "PlayerBike.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

PlayerBike::PlayerBike() {
    // Apoyado directamente sobre la superficie de la pista
    position = glm::vec3(0.0f, 0.20f, 0.0f);
    rotation = 0.0f;
    speed = 8.5f;
    turnSpeed = 65.0f;
    wheelRotation = 0.0f;

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
    bikerModel = new Model((char*)"../assets/obj/ciclista1/modelado-ciclista-blender.obj");
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
}

void PlayerBike::Draw(Shader& shader, GLint modelLoc, bool firstPerson) {
    // 1. Matriz raíz: posición en el mundo y dirección de manejo
    glm::mat4 root(1.0f);
    root = glm::translate(root, position);
    root = glm::rotate(root, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    root = glm::scale(root, glm::vec3(0.35f, 0.35f, 0.35f));

    // 2. Chasis de la bicicleta
    glm::mat4 modelBike = root;
    // Solo la bicicleta necesita corrección de 90° por cómo se exportó de Blender
    modelBike = glm::rotate(modelBike, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBike));
    if (bikeModel) bikeModel->Draw(shader);

    // 3. Bolsa / Maleta (Se empuja hacia adelante en -Z y arriba en Y)
    glm::mat4 modelBag = root;
    modelBag = glm::translate(modelBag, glm::vec3(0.0f, 0.95f, -0.60f));
    modelBag = glm::scale(modelBag, glm::vec3(0.50f, 0.50f, 0.50f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBag));
    if (backpackModel) backpackModel->Draw(shader);

    // 4. Ciclista y Casco
    if (!firstPerson) {
        // Ciclista: Se centra en X (0.0), se baja a los pedales (-Y) y se retrasa al asiento (+Z)
        glm::mat4 modelBiker = root;
        modelBiker = glm::translate(modelBiker, glm::vec3(0.0f, -0.65f, 0.15f));
        modelBiker = glm::rotate(modelBiker, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBiker));
        if (bikerModel) bikerModel->Draw(shader);

        // Casco: Se alinea con la cabeza del ciclista en Z y se sube en Y
        glm::mat4 modelHelmet = root;
        modelHelmet = glm::translate(modelHelmet, glm::vec3(0.0f, 1.25f, 0.15f));
        modelHelmet = glm::scale(modelHelmet, glm::vec3(0.22f, 0.22f, 0.22f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelHelmet));
        if (helmetModel) helmetModel->Draw(shader);
    }
}

glm::vec3 PlayerBike::GetFrontVector() {
    return glm::vec3(-sin(glm::radians(rotation)), 0.0f, -cos(glm::radians(rotation)));
}