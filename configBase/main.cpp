#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "PlayerBike.h"
#include "CicloviaScene.h"

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void ResetToStart();
void PrintPosition();

const GLuint WIDTH = 1280, HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;

Camera camera(glm::vec3(0.0f, 1.5f, 3.0f));
PlayerBike player;
CicloviaScene scene;   // incluye Polanco (zona urbana) en el mismo mundo

// Faro de la bici (es de dia, asi que casi no se necesita). 0 = apagado, 1 = intensidad original.
const float HEADLIGHT = 0.25f;

// Direccion del sol (inclinado, para que las fachadas reciban luz y se noten los volumenes)
const glm::vec3 SUN_DIR(-0.25f, -0.75f, -0.60f);

// Inicio de la pista (se guarda despues de player.Init())
glm::vec3 startPosition(0.0f);
float startRotation = 0.0f;

GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool keys[1024];
bool firstMouse = true;

// Estado de camara:
// 0 = Primera Persona GoPro
// 1 = Tercera Persona Persecucion
// 2 = Camara Libre de depuracion
int cameraMode = 0;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ciclovia Ferrocarril de Cuernavaca - OpenGL", nullptr, nullptr);
    if (!window) {
        std::cout << "Error al crear la ventana GLFW" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Error al inicializar GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);
  

    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");

    player.Init();
    startPosition = player.position;
    startRotation = player.rotation;
    scene.Init();          // carga la Ciclovia y Polanco

    ResetToStart();        // deja la camara libre lista en el inicio de la pista

    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

    // FOV amplio de 65 grados (lente angular de GoPro). Plano lejano 500 para ver los edificios del fondo.
    glm::mat4 projection = glm::perspective(glm::radians(65.0f), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 500.0f);

    while (!glfwWindowShouldClose(window)) {
        GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        // Cielo azul claro
        glClearColor(0.50f, 0.72f, 0.88f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightingShader.Use();

        // 1. Luz del Sol
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), SUN_DIR.x, SUN_DIR.y, SUN_DIR.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.38f, 0.40f, 0.45f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.85f, 0.82f, 0.72f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.20f, 0.20f, 0.20f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);

        // 2. Faro frontal
        glm::vec3 front = player.GetFrontVector();
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), player.position.x, player.position.y + 0.6f, player.position.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), front.x, -0.1f, front.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.1f * HEADLIGHT, 0.1f * HEADLIGHT, 0.1f * HEADLIGHT);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.9f * HEADLIGHT, 0.9f * HEADLIGHT, 0.8f * HEADLIGHT);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.4f * HEADLIGHT, 0.4f * HEADLIGHT, 0.4f * HEADLIGHT);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.07f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.017f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(15.0f)));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(22.0f)));

        // 3. Sistema de camaras
        glm::mat4 view;
        glm::vec3 eyePos;

        if (cameraMode == 0) {
            // MODO 1: GoPro sobre el manillar
            glm::vec3 headOffset = glm::vec3(-sin(glm::radians(player.rotation)) * 0.15f, 0.75f, -cos(glm::radians(player.rotation)) * 0.15f);
            eyePos = player.position + headOffset;
            glm::vec3 target = eyePos + front + glm::vec3(0.0f, -0.15f, 0.0f);
            view = glm::lookAt(eyePos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if (cameraMode == 1) {
            // MODO 2: Tercera persona de persecucion
            glm::vec3 followOffset = glm::vec3(sin(glm::radians(player.rotation)) * 1.8f, 1.1f, cos(glm::radians(player.rotation)) * 1.8f);
            eyePos = player.position + followOffset;
            glm::vec3 target = player.position + glm::vec3(0.0f, 0.55f, 0.0f);
            view = glm::lookAt(eyePos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else {
            // MODO 3: Camara libre
            view = camera.GetViewMatrix();
            eyePos = camera.GetPosition();
        }

        glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"), eyePos.x, eyePos.y, eyePos.z);

        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
        GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // 4. Renderizado del mundo (Polanco + ascenso rural + pueblo)
        scene.Draw(lightingShader, modelLoc);

        player.Draw(lightingShader, modelLoc, (cameraMode == 0));

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void DoMovement() {
    player.ProcessInput(keys, deltaTime);

    if (cameraMode == 2) {
        if (keys[GLFW_KEY_I]) camera.ProcessKeyboard(FORWARD, deltaTime);
        if (keys[GLFW_KEY_K]) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (keys[GLFW_KEY_J]) camera.ProcessKeyboard(LEFT, deltaTime);
        if (keys[GLFW_KEY_L]) camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }

    // Tecla C: GoPro -> Persecucion -> Camara libre
    if (keys[GLFW_KEY_C] && action == GLFW_PRESS) {
        cameraMode = (cameraMode + 1) % 3;
    }

    // Tecla 2: regresar al inicio de la pista (Polanco)
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        ResetToStart();
    }

    // Tecla P: imprime tu posicion en la consola
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        PrintPosition();
    }
    // Tecla H: vuelve a mostrar los controles
    
}

// Regresa la bici (y la camara libre) al inicio de la pista
void ResetToStart() {
    player.position = startPosition;
    player.rotation = startRotation;

    // Camara libre (modo 3 con la tecla C): 4 m detras de la bici, un poco arriba, mirando hacia donde mira la bici
    glm::vec3 f = player.GetFrontVector();
    float yaw = glm::degrees(std::atan2(f.z, f.x));
    camera.SetView(startPosition - f * 4.0f + glm::vec3(0.0f, 2.5f, 0.0f), yaw, -12.0f);

    std::cout << "[Ubicacion] Inicio de la pista (Polanco)" << std::endl;
}

void PrintPosition() {
    std::cout << "[Posicion] bici=(" << player.position.x << ", " << player.position.y << ", " << player.position.z << ")"
        << "  rotacion=" << player.rotation << " grados" << std::endl;
    if (cameraMode == 2) {
        glm::vec3 c = camera.GetPosition();
        std::cout << "           camara libre=(" << c.x << ", " << c.y << ", " << c.z << ")" << std::endl;
    }
}



void MouseCallback(GLFWwindow* window, double xPos, double yPos) {
    if (firstMouse) {
        lastX = static_cast<GLfloat>(xPos);
        lastY = static_cast<GLfloat>(yPos);
        firstMouse = false;
    }

    GLfloat xOffset = static_cast<GLfloat>(xPos) - lastX;
    GLfloat yOffset = lastY - static_cast<GLfloat>(yPos);
    lastX = static_cast<GLfloat>(xPos);
    lastY = static_cast<GLfloat>(yPos);

    if (cameraMode == 2) {
        camera.ProcessMouseMovement(xOffset, yOffset);
    }
}