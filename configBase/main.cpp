#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "PlayerBike.h"
#include "CicloviaScene.h"
#include "GameLogic.h"
#include "Ruta.h"

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void ResetToStart();
void PrintPosition();
void InitHUD();
void RenderHUD(float psi, float maxPsi, float agua, float maxAgua, int screenWidth, int screenHeight);

const GLuint WIDTH = 1280, HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;

Camera camera(glm::vec3(0.0f, 1.5f, 3.0f));
PlayerBike player;
CicloviaScene scene;   // Incluye Polanco y zona rural
GameLogic game;        // Gestor de mecánicas, presiones y coleccionables

// Faro frontal de la bicicleta
const float HEADLIGHT = 0.25f;

// Dirección del sol
const glm::vec3 SUN_DIR(-0.25f, -0.75f, -0.60f);

// Inicio de la pista
glm::vec3 startPosition(0.0f);
float startRotation = 0.0f;

GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool keys[1024];
bool firstMouse = true;

// Estado de cámara:
// 0 = Primera Persona GoPro
// 1 = Tercera Persona Persecución
// 2 = Cámara Libre de depuración
int cameraMode = 0;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// VAO/VBO y Shader para el HUD 2D
GLuint hudVAO = 0, hudVBO = 0;
GLuint hudProgram = 0;

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
    scene.Init();          // Carga la Ciclovia y Polanco
    InitHUD();             // Inicializa la geometría de las barras del HUD
    game.Init();
    ResetToStart();

    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

    glm::mat4 projection = glm::perspective(glm::radians(65.0f), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 500.0f);

    while (!glfwWindowShouldClose(window)) {
        GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        // Color de cielo con destello rojizo si se atropella un clavo
        if (game.alertaDanioTimer > 0.0f) {
            glClearColor(0.85f, 0.35f, 0.30f, 1.0f);
        }
        else {
            glClearColor(0.50f, 0.72f, 0.88f, 1.0f);
        }
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

        // 3. Sistema de cámaras
        glm::mat4 view;
        glm::vec3 eyePos;

        if (cameraMode == 0) {
            // GoPro sobre el manillar
            glm::vec3 headOffset = glm::vec3(-sin(glm::radians(player.rotation)) * 0.15f, 0.75f, -cos(glm::radians(player.rotation)) * 0.15f);
            eyePos = player.position + headOffset;
            glm::vec3 target = eyePos + front + glm::vec3(0.0f, -0.15f, 0.0f);
            view = glm::lookAt(eyePos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if (cameraMode == 1) {
            // Tercera persona
            glm::vec3 followOffset = glm::vec3(sin(glm::radians(player.rotation)) * 1.8f, 1.1f, cos(glm::radians(player.rotation)) * 1.8f);
            eyePos = player.position + followOffset;
            glm::vec3 target = player.position + glm::vec3(0.0f, 0.55f, 0.0f);
            view = glm::lookAt(eyePos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else {
            // Cámara libre
            view = camera.GetViewMatrix();
            eyePos = camera.GetPosition();
        }

        glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"), eyePos.x, eyePos.y, eyePos.z);

        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
        GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // 4. Renderizado del mundo y fauna
        scene.Draw(lightingShader, modelLoc);

        // 5. Renderizado de las entidades del juego (ánforas, clavos, talleres)
        game.DrawEntidades(lightingShader, modelLoc);

        // 6. Renderizado de la bicicleta
        player.Draw(lightingShader, modelLoc, (cameraMode == 0));

        // 7. Renderizado del HUD en 2D (Barras de PSI y Agua)
        RenderHUD(game.psi, game.PSI_MAX, game.agua, game.AGUA_MAX, SCREEN_WIDTH, SCREEN_HEIGHT);

        glfwSwapBuffers(window);
    }

    if (hudVAO) glDeleteVertexArrays(1, &hudVAO);
    if (hudVBO) glDeleteBuffers(1, &hudVBO);
    if (hudProgram) glDeleteProgram(hudProgram);

    glfwTerminate();
    return 0;
}

void DoMovement() {
    // 1. Cálculo de velocidad real de la bicicleta según desplazamiento
    static glm::vec3 prevPos = player.position;
    float velocidadReal = glm::length(player.position - prevPos) / (deltaTime > 0.0001f ? deltaTime : 0.016f);
    prevPos = player.position;

    // 2. Proyección sobre la ruta (cálculo de sActual y lateralActual)
    const auto& muestras = Ruta::Muestras();
    static float sActual = 0.0f;
    float lateralActual = 0.0f;

    float menorDist = 1e9f;
    size_t mejorIdx = 0;

    // Búsqueda en ventana local (+-35m) para evitar que curvas cerradas o alta velocidad salten a la meta
    for (size_t i = 0; i < muestras.size(); i += 2) {
        float difS = std::abs(muestras[i].s - sActual);
        if (sActual > 5.0f && difS > 35.0f) continue;

        float d = glm::distance(glm::vec2(player.position.x, player.position.z),
            glm::vec2(muestras[i].pos.x, muestras[i].pos.z));
        if (d < menorDist) {
            menorDist = d;
            mejorIdx = i;
        }
    }

    // VALIDACIÓN DE ESPACIOS MUERTOS:
    // Si la bici se aleja más de 12 metros de la pista, se considera fuera de límites:
    // no avanza el progreso 's' y se fuerza un lateral alto para que el terreno castigue la presión.
    if (menorDist > 12.0f) {
        lateralActual = (lateralActual >= 0.0f) ? 15.0f : -15.0f;
    }
    else {
        sActual = muestras[mejorIdx].s;
        float th = glm::radians(muestras[mejorIdx].headingDeg);
        glm::vec2 right(std::cos(th), -std::sin(th));
        glm::vec2 delta(player.position.x - muestras[mejorIdx].pos.x, player.position.z - muestras[mejorIdx].pos.z);
        lateralActual = glm::dot(delta, right);
    }

    // 3. Actualizar la lógica del juego (desgaste de PSI, hidratación, recolección y meta)
    GameStatus estadoAnterior = game.estado;
    game.Actualizar(deltaTime, sActual, lateralActual, velocidadReal, 0.0f);

    // Detección de cambios de estado para avisos en terminal
    if (game.estado != estadoAnterior) {
        if (game.estado == STATUS_VICTORIA) {
            std::cout << "\n==========================================" << std::endl;
            std::cout << "  ¡LLEGASTE A TEPOZTLAN! (VICTORIA)" << std::endl;
            std::cout << "  Tiempo total: " << game.tiempoTotal << " s | Anforas: " << game.anforasRecogidas << std::endl;
            std::cout << "  Presiona 'R' para reiniciar el recorrido." << std::endl;
            std::cout << "==========================================\n" << std::endl;
        }
        else if (game.estado == STATUS_GAME_OVER_PONCHADURA) {
            std::cout << "\n[GAME OVER] Llanta ponchada (0 PSI). Presiona 'R' para reiniciar." << std::endl;
        }
        else if (game.estado == STATUS_GAME_OVER_DESHIDRATACION) {
            std::cout << "\n[GAME OVER] Deshidratacion (0% Agua). Presiona 'R' para reiniciar." << std::endl;
        }
    }

    // 4. Procesamiento de controles de la bicicleta
    if (game.estado == STATUS_GAME_OVER_PONCHADURA || game.estado == STATUS_GAME_OVER_DESHIDRATACION) {
        // En Game Over se bloquea el avance hasta presionar R
    }
    else {
        player.ProcessInput(keys, deltaTime);
    }

    // 5. Movimiento en modo cámara libre (Modo 2)
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

    // Tecla C: Alternar cámaras
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        cameraMode = (cameraMode + 1) % 3;
    }

    // Tecla 2 o R: Reiniciar partida y regresar al inicio
    if ((key == GLFW_KEY_2 || key == GLFW_KEY_R) && action == GLFW_PRESS) {
        ResetToStart();
    }

    // Tecla P: Imprimir telemetría en consola
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        PrintPosition();
    }
}

void ResetToStart() {
    player.position = startPosition;
    player.rotation = startRotation;

    // Reinicia contadores de juego, presiones y coleccionables
    game.Reiniciar();

    glm::vec3 f = player.GetFrontVector();
    float yaw = glm::degrees(std::atan2(f.z, f.x));
    camera.SetView(startPosition - f * 4.0f + glm::vec3(0.0f, 2.5f, 0.0f), yaw, -12.0f);

    std::cout << "\n==========================================" << std::endl;
    std::cout << " [INICIO] Rally CDMX a Tepoztlan" << std::endl;
    std::cout << " - Presion: 35 PSI | Hidratacion: 100%" << std::endl;
    std::cout << " - Cuida tus llantas de los clavos y el manto arido." << std::endl;
    std::cout << "==========================================\n" << std::endl;
}

void PrintPosition() {
    std::cout << "[Telemetria] Posicion=(" << player.position.x << ", "
        << player.position.y << ", " << player.position.z << ") | Presion="
        << game.psi << " PSI | Agua=" << game.agua << "%" << std::endl;
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

// ==============================================================================
// SISTEMA DE HUD 2D (BARRAS DE ESTADO IN-GAME)
// ==============================================================================
void InitHUD() {
    float quadVertices[] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };

    glGenVertexArrays(1, &hudVAO);
    glGenBuffers(1, &hudVBO);
    glBindVertexArray(hudVAO);
    glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    const char* vShaderSource =
        "#version 330 core\n"
        "layout (location = 0) in vec2 vertex;\n"
        "uniform mat4 projection;\n"
        "uniform mat4 model;\n"
        "void main() {\n"
        "    gl_Position = projection * model * vec4(vertex, 0.0, 1.0);\n"
        "}\0";

    const char* fShaderSource =
        "#version 330 core\n"
        "out vec4 color;\n"
        "uniform vec4 uiColor;\n"
        "void main() {\n"
        "    color = uiColor;\n"
        "}\0";

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderSource, NULL);
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderSource, NULL);
    glCompileShader(fragment);

    hudProgram = glCreateProgram();
    glAttachShader(hudProgram, vertex);
    glAttachShader(hudProgram, fragment);
    glLinkProgram(hudProgram);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void RenderHUD(float psi, float maxPsi, float agua, float maxAgua, int screenWidth, int screenHeight) {
    glDisable(GL_DEPTH_TEST);
    glUseProgram(hudProgram);

    glm::mat4 proj = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight);
    glUniformMatrix4fv(glGetUniformLocation(hudProgram, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
    GLint modelLoc = glGetUniformLocation(hudProgram, "model");
    GLint colorLoc = glGetUniformLocation(hudProgram, "uiColor");

    glBindVertexArray(hudVAO);

    auto drawBarra = [&](float x, float y, float w, float h, glm::vec4 col) {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, glm::vec3(x, y, 0.0f));
        m = glm::scale(m, glm::vec3(w, h, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        glUniform4fv(colorLoc, 1, glm::value_ptr(col));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        };

    const float MARGEN_X = 35.0f;
    const float ANCHO_TOTAL = 220.0f;
    const float ALTO_BARRA = 16.0f;

    // 1. Barra de Presión de Llantas (PSI)
    drawBarra(MARGEN_X - 2.0f, 65.0f - 2.0f, ANCHO_TOTAL + 4.0f, ALTO_BARRA + 4.0f, glm::vec4(0.1f, 0.1f, 0.1f, 0.8f));
    float pctPsi = std::max(0.0f, std::min(1.0f, psi / maxPsi));
    glm::vec4 colPsi = (pctPsi > 0.45f) ? glm::vec4(0.2f, 0.85f, 0.25f, 1.0f) : (pctPsi > 0.20f ? glm::vec4(0.95f, 0.75f, 0.1f, 1.0f) : glm::vec4(0.9f, 0.15f, 0.15f, 1.0f));
    drawBarra(MARGEN_X, 65.0f, ANCHO_TOTAL * pctPsi, ALTO_BARRA, colPsi);

    // 2. Barra de Hidratación (Agua)
    drawBarra(MARGEN_X - 2.0f, 35.0f - 2.0f, ANCHO_TOTAL + 4.0f, ALTO_BARRA + 4.0f, glm::vec4(0.1f, 0.1f, 0.1f, 0.8f));
    float pctAgua = std::max(0.0f, std::min(1.0f, agua / maxAgua));
    drawBarra(MARGEN_X, 35.0f, ANCHO_TOTAL * pctAgua, ALTO_BARRA, glm::vec4(0.15f, 0.60f, 0.95f, 1.0f));

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}