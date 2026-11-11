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
#include "Skybox.h"
#include "Texto.h"
#include "Semaforo.h"

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
CicloviaScene scene;
GameLogic game;
Skybox skybox;

const float HEADLIGHT = 0.25f;
const glm::vec3 SUN_DIR(-0.25f, -0.75f, -0.60f);

glm::vec3 startPosition(0.0f);
float startRotation = 0.0f;

GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool keys[1024];
bool firstMouse = true;

int cameraMode = 0; // 0 = GoPro, 1 = Tercera Persona, 2 = Libre

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
float sJugador = 0.0f;
int zonaActual = -1;
float zonaTimer = 0.0f;
bool enMenu = true;
bool modoExploracionTepoz = false;

// Funcionalidades de la PR
int multasSemaforo = 0;
float avisoTimer = 0.0f;
std::string avisoTexto;
bool tourAuto = false;
float tourT = 0.0f;

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
    scene.Init();
    if (!skybox.Init("../assets/skybox")) {
        std::cout << "No se pudo cargar el cielo (revisa assets/skybox)" << std::endl;
    }
    Texto::Init();
    InitHUD();
    game.Init();
    ResetToStart();

    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "envMap"), 5);
    glUniform1f(glGetUniformLocation(lightingShader.Program, "reflectivity"), 0.0f);

    glm::mat4 projection = glm::perspective(glm::radians(65.0f), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 500.0f);

    while (!glfwWindowShouldClose(window)) {
        GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        DoMovement();

        if (game.alertaDanioTimer > 0.0f) {
            glClearColor(0.85f, 0.35f, 0.30f, 1.0f);
        }
        else {
            glClearColor(0.50f, 0.72f, 0.88f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightingShader.Use();

        // 1. Sol
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), SUN_DIR.x, SUN_DIR.y, SUN_DIR.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.38f, 0.40f, 0.45f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.85f, 0.82f, 0.72f);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.20f, 0.20f, 0.20f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);

        // 2. Faro
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

        // 3. Sistema de Cámaras
        glm::mat4 view;
        glm::vec3 eyePos;

        int planoTour = ((int)(tourT / 12.0f)) % 5;
        bool primeraPersona = tourAuto ? (planoTour == 4) : (cameraMode == 0);

        if (tourAuto && planoTour != 4) {
            float r = glm::radians(player.rotation);
            glm::vec3 fwd(-std::sin(r), 0.0f, -std::cos(r));
            glm::vec3 der(std::cos(r), 0.0f, -std::sin(r));
            glm::vec3 centro = player.position + glm::vec3(0.0f, 0.7f, 0.0f);
            if (planoTour == 0)      eyePos = player.position - fwd * 2.8f + glm::vec3(0.0f, 1.4f, 0.0f);
            else if (planoTour == 1) eyePos = centro + der * 4.5f + fwd * 1.0f + glm::vec3(0.0f, 0.6f, 0.0f);
            else if (planoTour == 2) eyePos = player.position - fwd * 9.0f + glm::vec3(0.0f, 6.0f, 0.0f);
            else                     eyePos = centro + fwd * 6.0f + glm::vec3(0.0f, 0.8f, 0.0f);
            view = glm::lookAt(eyePos, centro, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if (primeraPersona) {
            glm::vec3 headOffset = glm::vec3(-sin(glm::radians(player.rotation)) * 0.15f, 0.75f, -cos(glm::radians(player.rotation)) * 0.15f);
            eyePos = player.position + headOffset;
            glm::vec3 target = eyePos + front + glm::vec3(0.0f, -0.15f, 0.0f);
            view = glm::lookAt(eyePos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if (cameraMode == 1) {
            glm::vec3 followOffset = glm::vec3(sin(glm::radians(player.rotation)) * 1.8f, 1.1f, cos(glm::radians(player.rotation)) * 1.8f);
            eyePos = player.position + followOffset;
            glm::vec3 target = player.position + glm::vec3(0.0f, 0.55f, 0.0f);
            view = glm::lookAt(eyePos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else {
            view = camera.GetViewMatrix();
            eyePos = camera.GetPosition();
        }

        glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"), eyePos.x, eyePos.y, eyePos.z);

        // Niebla del bosque: aparece al entrar (s 210) y se disipa antes del árido (s 320)
        auto suave = [](float a, float b, float x) {
            float t = glm::clamp((x - a) / (b - a), 0.0f, 1.0f);
            return t * t * (3.0f - 2.0f * t);
            };
        float factorNiebla = suave(205.0f, 222.0f, sJugador) * (1.0f - suave(285.0f, 318.0f, sJugador));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "nieblaDensidad"), 0.022f * factorNiebla);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "nieblaColor"), 0.72f, 0.78f, 0.80f);

        GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
        GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        skybox.Draw(view, projection);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.cubemap);
        glActiveTexture(GL_TEXTURE0);

        lightingShader.Use();

        // 4. Renderizado de escena
        scene.Draw(lightingShader, modelLoc);
        game.DrawEntidades(lightingShader, modelLoc);
        player.Draw(lightingShader, modelLoc, primeraPersona);

        // 5. Renderizado HUD
        RenderHUD(game.psi, game.PSI_MAX, game.agua, game.AGUA_MAX, SCREEN_WIDTH, SCREEN_HEIGHT);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        {
            const int W = SCREEN_WIDTH, H = SCREEN_HEIGHT;
            const glm::vec4 BLANCO(1.0f), AMARILLO(1.0f, 0.85f, 0.2f, 1.0f);
            auto centrado = [&](const std::string& s, float y, float esc, glm::vec4 c) {
                Texto::Dibujar(s, (W - Texto::Ancho(s, esc)) * 0.5f, y, esc, c, W, H);
                };

            Texto::Dibujar("LLANTAS (PSI)", 35.0f, 86.0f, 2.0f, BLANCO, W, H);
            Texto::Dibujar("HIDRATACION", 35.0f, 56.0f, 2.0f, BLANCO, W, H);

            const char* nombres[5] = {
                "ZONA 1 - URBANA: POLANCO",
                "ZONA 2 - RURAL: BOSQUE DE OCOTES",
                "ZONA 2 - RURAL: BIOMA ARIDO",
                "ZONA 2 - RURAL: TERRACERIA",
                "ZONA 3 - TEPOZTLAN" };
            const char* tareas[5] = {
                "TAREA: RESPETA EL SEMAFORO Y RECOGE AGUA",
                "TAREA: ESQUIVA LOS CLAVOS Y RECOGE AGUA",
                "TAREA: CRUZA EL TEPETATE Y CUIDA TU HIDRATACION",
                "TAREA: LA TERRACERIA DESINFLA LAS LLANTAS",
                "TAREA: LLEGA A LA PLAZA DE TEPOZTLAN" };
            int z = (zonaActual < 0) ? 0 : zonaActual;
            Texto::Panel(20.0f, H - 110.0f, 640.0f, 95.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.45f), W, H);
            Texto::Dibujar(nombres[z], 32.0f, H - 40.0f, 3.0f, AMARILLO, W, H);
            Texto::Dibujar(tareas[z], 32.0f, H - 66.0f, 2.0f, BLANCO, W, H);
            Texto::Dibujar("RECORRIDO: " + std::to_string((int)sJugador) + " / 530 M  ANFORAS: "
                + std::to_string(game.anforasRecogidas) + "/" + std::to_string((int)game.anforas.size())
                + "  MULTAS: " + std::to_string(multasSemaforo),
                32.0f, H - 92.0f, 2.0f, BLANCO, W, H);

            // Aviso grande de multa
            if (avisoTimer > 0.0f && game.estado == STATUS_JUGANDO) {
                centrado(avisoTexto, H * 0.45f, 4.0f, glm::vec4(1.0f, 0.3f, 0.3f, std::min(1.0f, avisoTimer)));
            }

            // Minimapa
            {
                const float MX = W - 120.0f, MY = H - 275.0f, MW = 100.0f, MH = 255.0f;
                const float XMIN = -138.0f, ESC = 0.36f;
                auto aMapa = [&](float x, float z) {
                    return glm::vec2(MX + 18.0f + (x - XMIN) * ESC, MY + 12.0f + (-z) * ESC);
                    };

                static std::vector<glm::vec2> mapaUrbana, mapaRural, mapaPueblo;
                if (mapaUrbana.empty()) {
                    const auto& m = Ruta::Muestras();
                    for (size_t i = 0; i < m.size(); i += 4) {
                        glm::vec2 p = aMapa(m[i].pos.x, m[i].pos.z);
                        if (m[i].s < 210.0f) mapaUrbana.push_back(p);
                        else if (m[i].s < 475.0f) mapaRural.push_back(p);
                        else mapaPueblo.push_back(p);
                    }
                }

                Texto::Panel(MX, MY, MW, MH, glm::vec4(0.0f, 0.0f, 0.0f, 0.45f), W, H);
                Texto::Cuadros(mapaUrbana, 3.0f, glm::vec4(0.35f, 0.65f, 1.0f, 1.0f), W, H);
                Texto::Cuadros(mapaRural, 3.0f, glm::vec4(0.40f, 0.85f, 0.40f, 1.0f), W, H);
                Texto::Cuadros(mapaPueblo, 3.0f, glm::vec4(1.0f, 0.65f, 0.25f, 1.0f), W, H);

                std::vector<glm::vec2> mSemaforos, mAnforas;
                for (float sSem : Semaforo::POSICIONES) {
                    glm::vec3 q = Ruta::En(sSem).pos;
                    mSemaforos.push_back(aMapa(q.x, q.z));
                }
                for (const auto& a : game.anforas) {
                    if (a.recogida) continue;
                    glm::vec3 q = Ruta::En(a.s).pos;
                    mAnforas.push_back(aMapa(q.x, q.z));
                }
                Texto::Cuadros(mSemaforos, 5.0f, glm::vec4(1.0f, 0.2f, 0.2f, 1.0f), W, H);
                Texto::Cuadros(mAnforas, 4.0f, glm::vec4(0.3f, 0.9f, 1.0f, 1.0f), W, H);

                std::vector<glm::vec2> mJugador = { aMapa(player.position.x, player.position.z) };
                Texto::Cuadros(mJugador, 7.0f, AMARILLO, W, H);

                Texto::Dibujar("MAPA", MX + 30.0f, MY - 18.0f, 2.0f, BLANCO, W, H);
            }

            if (zonaTimer > 0.0f && game.estado == STATUS_JUGANDO) {
                float a = std::min(1.0f, zonaTimer);
                centrado(nombres[z], H * 0.62f, 5.0f, glm::vec4(1.0f, 0.85f, 0.2f, a));
            }

            // Fin de Juego / Victoria con Estadísticas Completas
            if (game.estado != STATUS_JUGANDO) {
                const glm::vec4 VERDE(0.3f, 1.0f, 0.4f, 1.0f), ROJO(1.0f, 0.3f, 0.3f, 1.0f);
                if (game.estado == STATUS_VICTORIA) {
                    int puntos = 1000 + game.anforasRecogidas * 50 + (int)game.psi * 10 + (int)game.agua * 5
                        - multasSemaforo * 100 - game.clavosAtropellados * 50 - (int)game.tiempoTotal * 2;
                    if (puntos < 0) puntos = 0;
                    std::string calif = (puntos >= 1400) ? "EXCELENTE" : (puntos >= 1100) ? "MUY BIEN" : (puntos >= 800) ? "BIEN" : "A MEJORAR";

                    Texto::Panel(0.0f, H * 0.15f, (float)W, H * 0.70f, glm::vec4(0.0f, 0.0f, 0.0f, 0.75f), W, H);
                    centrado("LLEGASTE A TEPOZTLAN!", H * 0.74f, 7.0f, VERDE);
                    centrado("RESUMEN DEL RECORRIDO", H * 0.66f, 3.0f, AMARILLO);

                    float x0 = W * 0.5f - 260.0f, y0 = H * 0.58f, dy = 30.0f;
                    Texto::Dibujar("TIEMPO ................ " + std::to_string((int)game.tiempoTotal) + " S", x0, y0, 2.5f, BLANCO, W, H);
                    Texto::Dibujar("ANFORAS RECOGIDAS ..... " + std::to_string(game.anforasRecogidas) + "/" + std::to_string((int)game.anforas.size()), x0, y0 - dy, 2.5f, BLANCO, W, H);
                    Texto::Dibujar("CLAVOS ATROPELLADOS ... " + std::to_string(game.clavosAtropellados), x0, y0 - 2 * dy, 2.5f, BLANCO, W, H);
                    Texto::Dibujar("MULTAS DE SEMAFORO .... " + std::to_string(multasSemaforo), x0, y0 - 3 * dy, 2.5f, BLANCO, W, H);
                    Texto::Dibujar("LLANTAS AL LLEGAR ..... " + std::to_string((int)game.psi) + " PSI", x0, y0 - 4 * dy, 2.5f, BLANCO, W, H);
                    Texto::Dibujar("HIDRATACION ........... " + std::to_string((int)game.agua) + " %", x0, y0 - 5 * dy, 2.5f, BLANCO, W, H);

                    centrado("PUNTOS: " + std::to_string(puntos) + "   " + calif, H * 0.30f, 4.0f, AMARILLO);

                    // Botón para modo exploración
                    float btnW = 380.0f, btnH = 38.0f;
                    float btnX = (W - btnW) * 0.5f;
                    float btnY = H * 0.22f;
                    Texto::Panel(btnX, btnY, btnW, btnH, glm::vec4(0.12f, 0.50f, 0.90f, 0.90f), W, H);
                    centrado("MODO TURISTA [ ENTER ]", btnY + 10.0f, 2.3f, BLANCO);

                    centrado("PRESIONA R PARA REINICIAR", H * 0.16f, 2.5f, BLANCO);
                }
                else {
                    Texto::Panel(0.0f, H * 0.35f, (float)W, H * 0.30f, glm::vec4(0.0f, 0.0f, 0.0f, 0.65f), W, H);
                    centrado(game.estado == STATUS_GAME_OVER_PONCHADURA ? "LLANTA PONCHADA" : "DESHIDRATACION",
                        H * 0.52f, 7.0f, ROJO);
                    centrado("PRESIONA R PARA REINICIAR", H * 0.39f, 3.0f, BLANCO);
                }
            }

            // Menú de Inicio / Ayuda
            if (enMenu) {
                Texto::Panel(0.0f, 0.0f, (float)W, (float)H, glm::vec4(0.0f, 0.0f, 0.0f, 0.78f), W, H);
                centrado("CICLOVIA FERROCARRIL DE CUERNAVACA", H * 0.82f, 5.0f, AMARILLO);
                centrado("CDMX - TEPOZTLAN   |   EQUIPO 1 CICLOTURISMO", H * 0.75f, 2.5f, BLANCO);

                const char* controles[] = {
                    "W / FLECHA ARRIBA ... PEDALEAR",
                    "S / FLECHA ABAJO .... FRENAR",
                    "A D / FLECHAS ....... GIRAR",
                    "SHIFT ............... IMPULSO",
                    "C ................... CAMBIAR CAMARA",
                    "1 A 5 ............... SALTAR A UNA ZONA",
                    "R ................... REINICIAR",
                    "T ................... RECORRIDO AUTOMATICO",
                    "H ................... AYUDA",
                    "ESC ................. SALIR" };
                float yc = H * 0.64f;
                for (const char* linea : controles) {
                    Texto::Dibujar(linea, W * 0.5f - 270.0f, yc, 2.5f, BLANCO, W, H);
                    yc -= 28.0f;
                }

                centrado("OBJETIVO: LLEGA A TEPOZTLAN SIN PONCHAR NI DESHIDRATARTE", H * 0.25f, 2.5f, AMARILLO);
                centrado("RECOGE ANFORAS DE AGUA, ESQUIVA LOS CLAVOS E INFLA EN LOS TALLERES", H * 0.20f, 2.0f, BLANCO);

                float parpadeo = 0.55f + 0.45f * std::sin((float)glfwGetTime() * 4.0f);
                centrado(game.tiempoTotal > 0.0f ? "PRESIONA H PARA CONTINUAR" : "PRESIONA ENTER PARA COMENZAR",
                    H * 0.10f, 3.5f, glm::vec4(0.3f, 1.0f, 0.4f, parpadeo));
            }
        }

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
    }

    if (hudVAO) glDeleteVertexArrays(1, &hudVAO);
    if (hudVBO) glDeleteBuffers(1, &hudVBO);
    if (hudProgram) glDeleteProgram(hudProgram);
    Texto::Fin();

    glfwTerminate();
    return 0;
}

void DoMovement() {
    if (enMenu) return;
    static glm::vec3 prevPos = player.position;
    float velocidadReal = glm::length(player.position - prevPos) / (deltaTime > 0.0001f ? deltaTime : 0.016f);
    prevPos = player.position;

    const auto& muestras = Ruta::Muestras();
    static float sActual = 0.0f;
    float lateralActual = 0.0f;

    float menorDist = 1e9f;
    size_t mejorIdx = 0;

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

    if (menorDist > 12.0f) {
        for (size_t i = 0; i < muestras.size(); i += 2) {
            float d = glm::distance(glm::vec2(player.position.x, player.position.z),
                glm::vec2(muestras[i].pos.x, muestras[i].pos.z));
            if (d < menorDist) {
                menorDist = d;
                mejorIdx = i;
            }
        }
    }

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

    GameStatus estadoAnterior = game.estado;
    float psiGuardado = game.psi;
    float aguaGuardada = game.agua;

    game.Actualizar(deltaTime, sActual, lateralActual, velocidadReal, 0.0f);

    if (modoExploracionTepoz) {
        game.estado = STATUS_JUGANDO;
        game.psi = psiGuardado;
        game.agua = aguaGuardada;
    }

    sJugador = sActual;

    // Multas de semáforo
    static float sAnterior = 0.0f;
    if (game.estado == STATUS_JUGANDO && !modoExploracionTepoz) {
        for (float sSem : Semaforo::POSICIONES) {
            if (sAnterior < sSem && sActual >= sSem && sActual - sAnterior < 20.0f
                && Semaforo::FaseEn((float)glfwGetTime()) == Semaforo::ROJO) {
                game.psi = std::max(0.0f, game.psi - 5.0f);
                multasSemaforo++;
                avisoTexto = "CRUZASTE EN ROJO! -5 PSI";
                avisoTimer = 3.0f;
            }
        }
    }
    sAnterior = sActual;
    if (avisoTimer > 0.0f) avisoTimer -= deltaTime;

    int zonaNueva = (sActual < 210.0f) ? 0 : (sActual < 320.0f) ? 1 : (sActual < 415.0f) ? 2 : (sActual < 475.0f) ? 3 : 4;
    if (zonaNueva != zonaActual) { zonaActual = zonaNueva; zonaTimer = 4.0f; }
    if (zonaTimer > 0.0f) zonaTimer -= deltaTime;

    if (game.estado != estadoAnterior) {
        if (game.estado == STATUS_VICTORIA) {
            std::cout << "\n==========================================" << std::endl;
            std::cout << "  ¡LLEGASTE A TEPOZTLAN! (VICTORIA)" << std::endl;
            std::cout << "  Tiempo total: " << game.tiempoTotal << " s | Anforas: " << game.anforasRecogidas << std::endl;
            std::cout << "  Presiona 'ENTER' para modo turista o 'R' para reiniciar." << std::endl;
            std::cout << "==========================================\n" << std::endl;
        }
        else if (game.estado == STATUS_GAME_OVER_PONCHADURA) {
            std::cout << "\n[GAME OVER] Llanta ponchada (0 PSI). Presiona 'R' para reiniciar." << std::endl;
        }
        else if (game.estado == STATUS_GAME_OVER_DESHIDRATACION) {
            std::cout << "\n[GAME OVER] Deshidratacion (0% Agua). Presiona 'R' para reiniciar." << std::endl;
        }
    }

    // Controles y Piloto Automático
    if (game.estado == STATUS_GAME_OVER_PONCHADURA || game.estado == STATUS_GAME_OVER_DESHIDRATACION) {
        // Bloqueado
    }
    else if (tourAuto) {
        tourT += deltaTime;
        player.speed = 3.6f;
        static bool k[1024];
        std::fill(k, k + 1024, false);
        bool parar = (game.estado != STATUS_JUGANDO);

        for (float sSem : Semaforo::POSICIONES) {
            if (sActual > sSem - 7.0f && sActual < sSem && Semaforo::FaseEn((float)glfwGetTime()) != Semaforo::VERDE) {
                parar = true;
            }
        }

        static float esperaTaller = 0.0f;
        bool enTaller = false;
        for (const auto& tal : game.talleres) {
            if (sActual > tal.s - 1.0f && sActual < tal.s + 1.0f && game.psi < game.PSI_MAX - 0.5f) {
                enTaller = true;
            }
        }
        if (enTaller) esperaTaller += deltaTime; else esperaTaller = 0.0f;
        if (enTaller && esperaTaller < 4.0f) parar = true;

        if (!parar) k[GLFW_KEY_W] = true;

        Ruta::Muestra objetivo = Ruta::En(std::min(sActual + 6.0f, Ruta::Largo()));
        float rumbo = glm::degrees(std::atan2(-(objetivo.pos.x - player.position.x), -(objetivo.pos.z - player.position.z)));
        float dif = rumbo - player.rotation;
        while (dif > 180.0f) dif -= 360.0f;
        while (dif < -180.0f) dif += 360.0f;
        if (dif > 2.0f) k[GLFW_KEY_A] = true;
        else if (dif < -2.0f) k[GLFW_KEY_D] = true;

        player.ProcessInput(k, deltaTime);
    }
    else {
        player.speed = 6.0f;
        player.ProcessInput(keys, deltaTime);
    }

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

    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        cameraMode = (cameraMode + 1) % 3;
    }

    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        if (enMenu) {
            enMenu = false;
            zonaActual = -1;
        }
        else if (game.estado == STATUS_VICTORIA) {
            game.estado = STATUS_JUGANDO;
            modoExploracionTepoz = true;
        }
    }

    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        enMenu = !enMenu;
    }

    if (key == GLFW_KEY_T && action == GLFW_PRESS && !enMenu) {
        tourAuto = !tourAuto;
        if (tourAuto) {
            ResetToStart();
            tourT = 0.0f;
        }
    }

    if ((key == GLFW_KEY_2 || key == GLFW_KEY_R) && action == GLFW_PRESS) {
        ResetToStart();
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        PrintPosition();
    }
}

void ResetToStart() {
    player.position = startPosition;
    player.rotation = startRotation;

    game.Reiniciar();
    zonaActual = -1;
    multasSemaforo = 0;
    avisoTimer = 0.0f;
    modoExploracionTepoz = false;

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

    drawBarra(MARGEN_X - 2.0f, 65.0f - 2.0f, ANCHO_TOTAL + 4.0f, ALTO_BARRA + 4.0f, glm::vec4(0.1f, 0.1f, 0.1f, 0.8f));
    float pctPsi = std::max(0.0f, std::min(1.0f, psi / maxPsi));
    glm::vec4 colPsi = (pctPsi > 0.45f) ? glm::vec4(0.2f, 0.85f, 0.25f, 1.0f) : (pctPsi > 0.20f ? glm::vec4(0.95f, 0.75f, 0.1f, 1.0f) : glm::vec4(0.9f, 0.15f, 0.15f, 1.0f));
    drawBarra(MARGEN_X, 65.0f, ANCHO_TOTAL * pctPsi, ALTO_BARRA, colPsi);

    drawBarra(MARGEN_X - 2.0f, 35.0f - 2.0f, ANCHO_TOTAL + 4.0f, ALTO_BARRA + 4.0f, glm::vec4(0.1f, 0.1f, 0.1f, 0.8f));
    float pctAgua = std::max(0.0f, std::min(1.0f, agua / maxAgua));
    drawBarra(MARGEN_X, 35.0f, ANCHO_TOTAL * pctAgua, ALTO_BARRA, glm::vec4(0.15f, 0.60f, 0.95f, 1.0f));

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}