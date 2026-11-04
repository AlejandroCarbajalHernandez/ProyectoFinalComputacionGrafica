#pragma once

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Model.h"

class PlayerBike {
public:
    glm::vec3 position;
    float rotation;
    float speed;
    float turnSpeed;
    float wheelRotation;

    Model* bikeModel;
    Model* bikerModel;
    Model* helmetModel;
    Model* backpackModel;

    PlayerBike();
    ~PlayerBike();

    void Init();
    void ProcessInput(bool keys[], float deltaTime);
    void Draw(Shader& shader, GLint modelLoc, bool firstPerson);
    glm::vec3 GetFrontVector();
};