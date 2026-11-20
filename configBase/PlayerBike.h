// PlayerBike.h: la bicicleta del jugador y su ciclista.
// Aquí se declara qué datos tiene la bici y qué acciones puede hacer;
// el detalle de cada acción está en PlayerBike.cpp.
#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Model.h"

// La bicicleta con la que se juega: su posición, cómo se mueve y cómo se dibuja.
class PlayerBike {
public:
    // Dónde está la bici en el escenario
    glm::vec3 position;
    // Hacia dónde está mirando la bici (en grados)
    float rotation;
    // Qué tan rápido avanza
    float speed;
    // Qué tan rápido gira al dar vuelta
    float turnSpeed;
    // Cuánto han girado las ruedas, para animarlas al avanzar
    float wheelRotation;
    float pitch;   // inclinacion de la bici en subidas y bajadas (grados)

    // Modelos 3D que forman al jugador: la bici, el ciclista, el casco y la mochila
    Model* bikeModel;
    Model* bikerModel;
    Model* helmetModel;
    Model* backpackModel;

    // Crea la bici con sus valores iniciales y la libera de la memoria al terminar
    PlayerBike();
    ~PlayerBike();

    // Carga los modelos 3D y deja la bici lista para usarse
    void Init();
    // Lee las teclas oprimidas y mueve la bici (avanzar, frenar y girar)
    void ProcessInput(bool keys[], float deltaTime);
    // Dibuja la bici con el ciclista; cambia un poco según se vea en primera persona o desde afuera
    void Draw(Shader& shader, GLint modelLoc, bool firstPerson);
    // Regresa hacia dónde apunta el frente de la bici (lo usan la cámara y el faro)
    glm::vec3 GetFrontVector();
};