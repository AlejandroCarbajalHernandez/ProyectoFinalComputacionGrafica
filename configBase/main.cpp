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
#include "PolancoScene.h"
#include "GameLogic.h"
#include "Ruta.h"
#include "Skybox.h"
#include "Texto.h"
#include "Narracion.h"
#include "Semaforo.h"
#include "AudioAmbiente.h"

// Prototipos de funciones de control, ciclo de vida e interfaz 2D
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void DoMovement();
void ResetToStart();
void PrintPosition();
void InitHUD();
void RenderHUD(float psi, float maxPsi, float agua, float maxAgua, int screenWidth, int screenHeight);

// Dimensiones de la ventana
const GLuint WIDTH = 1280, HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Instancias globales de cámara, jugador, escenarios y lógica del juego
Camera camera(glm::vec3(0.0f, 1.5f, 3.0f));
PlayerBike player;
CicloviaScene scene;
GameLogic game;
Skybox skybox;

// Parámetros de iluminación ambiental (ciclo día/noche) y faro delantero
const float HEADLIGHT = 0.25f;
bool faroEncendido = true;
bool esDeNoche = false;
float factorDia = 1.0f; // 1 = día, 0 = noche (cambia suavemente)
const glm::vec3 SUN_DIR(-0.25f, -0.75f, -0.60f);

glm::vec3 startPosition(0.0f);
float startRotation = 0.0f;

// Variables de control de entrada (teclado y ratón)
GLfloat lastX = WIDTH / 2.0f;
GLfloat lastY = HEIGHT / 2.0f;
bool keys[1024];
bool firstMouse = true;

int cameraMode = 0; // 0 = GoPro, 1 = Tercera Persona, 2 = Libre, 3 = Aérea

// Variables de tiempo, progreso en ruta y estados de interfaz
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
float sJugador = 0.0f;
int zonaActual = -1;
float zonaTimer = 0.0f;
bool enMenu = true;
bool pausado = false;                       // Pausa del juego (tecla P o botón en pantalla)
float btnPausa[4] = { 0, 0, 0, 0 };         // x, y, ancho, alto del botón de pausa (coordenadas de pantalla, origen abajo-izquierda)
bool modoExploracionTepoz = false;

// Pantallas de inicio y versión del juego
bool enBienvenida = true;      // ventana de bienvenida al abrir el programa
bool partidaIniciada = false;  // false = se está eligiendo la versión; true = ya se empezó a jugar
int seleccionDif = 0;          // 0 = novato (antes básica), 1 = intermedia, 2 = avanzada (la que está resaltada en el menú)

// Ayudas contextuales (según la versión) y avisos de recuerdos turísticos
bool vistoAyuda[4] = { false, false, false, false };   // intermedia: ya se mostró la ayuda de botella, semáforo, bomba, clavo
bool avisoBotella[16], avisoSem[4], avisoBomba[8], avisoClavo[16];   // básica: ya se avisó de ese objeto en esta pasada
float consejoTimer = 0.0f;
std::string consejoTitulo, consejoTexto;
float recuerdoTimer = 0.0f;
std::string recuerdoTitulo, recuerdoTexto;
const char* NOMBRE_DIF[3] = { "NOVATO", "INTERMEDIA", "AVANZADA" };

// Muestra una ayuda corta. tipo: 0 botella, 1 semáforo, 2 bomba, 3 clavo.
// Básica: siempre. Intermedia: solo la primera vez de cada tipo. Avanzada: nunca.
void MostrarConsejo(int tipo, const std::string& titulo, const std::string& texto) {
    if (game.dificultad == DIF_AVANZADA) return;
    if (game.dificultad == DIF_INTERMEDIA) {
        if (vistoAyuda[tipo]) return;
        vistoAyuda[tipo] = true;
    }
    consejoTitulo = titulo;
    consejoTexto = texto;
    consejoTimer = 6.0f;
}

// Variables de infracciones de tránsito, notificaciones y recorrido automático
int multasSemaforo = 0;
float avisoTimer = 0.0f;
float fueraPistaTimer = 0.0f;   // segundos seguidos fuera de la pista (aviso de novato e intermedia)
float avisoPistaTimer = 0.0f;   // intermedia: duración del aviso (solo se muestra la primera vez)
bool vistoAvisoPista = false;   // intermedia: ya se avisó una vez en esta partida
std::string avisoTexto;
bool tourAuto = false;
float tourT = 0.0f;

// Recursos OpenGL para el HUD 2D
GLuint hudVAO = 0, hudVBO = 0;
GLuint hudProgram = 0;

int main() {
    // Inicialización de GLFW y configuración del contexto OpenGL 3.3 Core Profile
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
    Audio::Iniciar();   // sonido ambiental por zona (carpeta Sonidos)
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Registro de callbacks de teclado y ratón
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // El ratón solo se usa para mirar en la cámara libre (modo 2); en los demás modos queda visible y libre.
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Inicialización de GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Error al inicializar GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // Compilación del shader principal de iluminación y carga de activos 3D/2D
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

    // Configuración inicial de unidades de textura en el shader
    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "envMap"), 5);
    glUniform1f(glGetUniformLocation(lightingShader.Program, "reflectivity"), 0.0f);

    glm::mat4 projection = glm::perspective(glm::radians(65.0f), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 500.0f);

    // Bucle principal de renderizado (Game Loop)
    while (!glfwWindowShouldClose(window)) {
        // Cálculo de deltaTime para sincronizar físicas y animaciones
        GLfloat currentFrame = static_cast<GLfloat>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (pausado) deltaTime = 0.0f;   // en pausa no avanza el tiempo del juego
        Audio::Actualizar(zonaActual, enBienvenida || enMenu, pausado);   // ambiente según la zona

        glfwPollEvents();
        DoMovement();

        // Interpolación gradual de iluminación entre día y noche
        float objetivoDia = esDeNoche ? 0.0f : 1.0f;
        float pasoDia = deltaTime / 18.0f;   // 18 s para pasar de día a noche
        if (factorDia < objetivoDia) factorDia = std::min(objetivoDia, factorDia + pasoDia);
        else                         factorDia = std::max(objetivoDia, factorDia - pasoDia);

        // Limpieza de buffers con destello rojo temporal si el jugador recibe daño
        if (game.alertaDanioTimer > 0.0f) {
            glClearColor(0.85f, 0.35f, 0.30f, 1.0f);
        }
        else {
            glClearColor(0.50f, 0.72f, 0.88f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightingShader.Use();

        // 1. Sol (Luz direccional modulada por el ciclo día/noche)
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), SUN_DIR.x, SUN_DIR.y, SUN_DIR.z);
        glm::vec3 ambiente = glm::mix(glm::vec3(0.10f, 0.11f, 0.22f), glm::vec3(0.38f, 0.40f, 0.45f), factorDia);
        glm::vec3 difusa = glm::mix(glm::vec3(0.08f, 0.10f, 0.22f), glm::vec3(0.85f, 0.82f, 0.72f), factorDia);
        float especular = glm::mix(0.03f, 0.20f, factorDia);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), ambiente.x, ambiente.y, ambiente.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), difusa.x, difusa.y, difusa.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), especular, especular, especular);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 16.0f);

        // 2. Faro (Luz reflector acoplada al frente de la bicicleta)
        float faro = faroEncendido ? HEADLIGHT * (1.0f + 5.0f * (1.0f - factorDia)) : 0.0f;
        glm::vec3 front = player.GetFrontVector();
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), player.position.x, player.position.y + 0.6f, player.position.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), front.x, -0.1f, front.z);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.1f * faro, 0.1f * faro, 0.1f * faro);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.9f * faro, 0.9f * faro, 0.8f * faro);
        glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.4f * faro, 0.4f * faro, 0.4f * faro);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.07f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.017f);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(15.0f)));
        glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(22.0f)));

        // 3. Sistema de Cámaras (tour cinematográfico, primera persona, tercera persona y libre)
        glm::mat4 view;
        glm::vec3 eyePos;

        // Recorrido automático: cámara fija en tercera persona (sin cambios de plano)
        bool primeraPersona = tourAuto ? false : (cameraMode == 0);

        if (tourAuto) {
            float r = glm::radians(player.rotation);
            glm::vec3 fwd(-std::sin(r), 0.0f, -std::cos(r));
            eyePos = player.position - fwd * 5.0f + glm::vec3(0.0f, 3.0f, 0.0f);
            glm::vec3 target = player.position + fwd * 4.0f + glm::vec3(0.0f, 0.9f, 0.0f);
            view = glm::lookAt(eyePos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if (primeraPersona) {
            glm::vec3 headOffset = glm::vec3(-sin(glm::radians(player.rotation)) * 0.15f, 0.95f, -cos(glm::radians(player.rotation)) * 0.15f);
            eyePos = player.position + headOffset;
            glm::vec3 target = eyePos + front + glm::vec3(0.0f, 0.10f, 0.0f);    // mira un poco hacia arriba para ver edificios altos completos
            view = glm::lookAt(eyePos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if (cameraMode == 1) {
            // Tercera persona: cámara más alta y atrás, mirando un poco adelante de la bici para ver más camino y horizonte
            glm::vec3 followOffset = glm::vec3(sin(glm::radians(player.rotation)) * 5.0f, 3.4f, cos(glm::radians(player.rotation)) * 5.0f);
            eyePos = player.position + followOffset;
            glm::vec3 target = player.position + front * 4.0f + glm::vec3(0.0f, 0.9f, 0.0f);
            view = glm::lookAt(eyePos, target, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if (cameraMode == 3) {
            // Vista aérea: más arriba y atrás para ver el entorno completo del recorrido
            eyePos = player.position - front * 14.0f + glm::vec3(0.0f, 18.0f, 0.0f);
            view = glm::lookAt(eyePos, player.position, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else {
            view = camera.GetViewMatrix();
            eyePos = camera.GetPosition();
        }

        // Campo de visión: casi igual al normal en tercera persona (para que no se deforme) y un poco más abierto en la vista aérea
        {
            static float fov = 68.0f;
            float fovMeta = (tourAuto || cameraMode == 1) ? 68.0f : (cameraMode == 3 ? 75.0f : 68.0f);
            fov += (fovMeta - fov) * std::min(1.0f, 4.0f * deltaTime);
            projection = glm::perspective(glm::radians(fov), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 500.0f);
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

        // Renderizado del Skybox y enlace de su cubemap para reflejos ambientales
        skybox.shader->Use();
        glUniform1f(glGetUniformLocation(skybox.shader->Program, "brillo"), factorDia);

        skybox.Draw(view, projection);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.cubemap);
        glActiveTexture(GL_TEXTURE0);

        lightingShader.Use();

        // 4. Renderizado de escena 3D (entorno, ítems/obstáculos y ciclista)
        scene.Draw(lightingShader, modelLoc);
        game.DrawEntidades(lightingShader, modelLoc);
        player.Draw(lightingShader, modelLoc, primeraPersona);

        // 5. Renderizado HUD y superposición de interfaz 2D (telemetría, minimapa y menús)
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

            Texto::Dibujar("LLANTAS (PSI)", 35.0f, 96.0f, 2.0f, BLANCO, W, H);
            Texto::Dibujar("HIDRATACIÓN", 35.0f, 50.0f, 2.0f, BLANCO, W, H);

            // Vidas (cuadros rojos): se pierde una cuando se acaba el aire o el agua
            Texto::Dibujar("VIDAS", 35.0f, 132.0f, 2.0f, BLANCO, W, H);
            {
                std::vector<glm::vec2> vivas, perdidas;
                for (int i = 0; i < game.vidasMax; i++)
                    ((i < game.vidas) ? vivas : perdidas).push_back(glm::vec2(125.0f + i * 24.0f, 139.0f));
                Texto::Cuadros(perdidas, 16.0f, glm::vec4(0.35f, 0.35f, 0.35f, 0.9f), W, H);
                Texto::Cuadros(vivas, 16.0f, glm::vec4(1.0f, 0.25f, 0.25f, 1.0f), W, H);
            }

            // Cajas de aviso reutilizables (ayudas y recuerdos): panel centrado con título y texto ajustado
            auto cajaAviso = [&](const std::string& titulo, const std::string& texto, float yTop, float a, glm::vec4 colT) {
                const float ESC = 2.0f, LH = 26.0f, PAD = 12.0f;
                float ancho = std::min(760.0f, W - 40.0f);
                std::vector<std::string> ls = Narracion::Ajustar(Narracion::Preparar(texto), ancho - 2.0f * PAD, ESC);
                float alto = 2.0f * PAD + 30.0f + (float)ls.size() * LH;
                float x = (W - ancho) * 0.5f, y = yTop - alto;
                Texto::Panel(x, y, ancho, alto, glm::vec4(0.0f, 0.0f, 0.0f, 0.68f * a), W, H);
                float yt = y + alto - PAD - 18.0f;
                Texto::Dibujar(titulo, x + PAD, yt, 2.5f, glm::vec4(colT.r, colT.g, colT.b, a), W, H);
                float yl = yt - 30.0f;
                for (const std::string& l : ls) {
                    Texto::Dibujar(l, x + PAD, yl, ESC, glm::vec4(1.0f, 1.0f, 1.0f, a), W, H);
                    yl -= LH;
                }
                };

            // Panel superior de zona actual y objetivos del tramo
            const char* nombres[5] = {
                "ZONA 1 - URBANA: POLANCO",
                "ZONA 2 - RURAL: BOSQUE DE OCOTES",
                "ZONA 2 - RURAL: BIOMA ÁRIDO",
                "ZONA 2 - RURAL: TERRACERÍA",
                "ZONA 3 - TEPOZTLÁN" };
            const char* tareas[5] = {
                "TAREA: RESPETA EL SEMÁFORO, RECOGE BOTELLAS",
                "TAREA: ESQUIVA LOS CLAVOS Y RECOGE BOTELLAS",
                "TAREA: CRUZA EL TEPETATE Y CUIDA TU HIDRATACIÓN",
                "TAREA: LA TERRACERÍA DESINFLA LAS LLANTAS",
                "TAREA: LLEGA A LA PLAZA DE TEPOZTLÁN" };
            int z = (zonaActual < 0) ? 0 : zonaActual;
            Texto::Panel(20.0f, H - 134.0f, 640.0f, 119.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.45f), W, H);
            Texto::Dibujar(nombres[z], 32.0f, H - 40.0f, 3.0f, AMARILLO, W, H);
            Texto::Dibujar(tareas[z], 32.0f, H - 66.0f, 2.0f, BLANCO, W, H);
            Texto::Dibujar("RECORRIDO: " + std::to_string((int)sJugador) + " / 530 M  BOTELLAS: "
                + std::to_string(game.anforasRecogidas) + "/" + std::to_string((int)game.anforas.size())
                + "  MULTAS: " + std::to_string(multasSemaforo),
                32.0f, H - 92.0f, 2.0f, BLANCO, W, H);
            Texto::Dibujar("RECUERDOS: " + std::to_string(game.recuerdosRecogidos) + "/" + std::to_string((int)game.recuerdos.size())
                + "  VERSIÓN: " + (tourAuto ? std::string("DEMO") : std::string(NOMBRE_DIF[game.dificultad])),
                32.0f, H - 118.0f, 2.0f, glm::vec4(1.0f, 0.85f, 0.2f, 1.0f), W, H);

            // Versión avanzada: cuenta regresiva del tiempo límite
            if (game.tiempoLimite > 0.0f && game.estado == STATUS_JUGANDO) {
                float resta = std::max(0.0f, game.tiempoLimite - game.tiempoTotal);
                int m = (int)resta / 60, sg = (int)resta % 60;
                std::string txt = "TIEMPO " + std::to_string(m) + ":" + (sg < 10 ? "0" : "") + std::to_string(sg);
                bool urgente = resta < 30.0f;
                float parp = urgente ? (0.6f + 0.4f * std::sin((float)glfwGetTime() * 8.0f)) : 1.0f;
                Texto::Panel(680.0f, H - 100.0f, 190.0f, 40.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.55f), W, H);
                Texto::Dibujar(txt, 692.0f, H - 90.0f, 3.0f, urgente ? glm::vec4(1.0f, 0.3f, 0.3f, parp) : BLANCO, W, H);
            }

            // Caja de indicaciones / datos interesantes de la zona (abajo al centro)
            if (game.estado == STATUS_JUGANDO) Narracion::Dibujar(W, H);

            // Aviso de calor (arriba a la derecha): en el árido y la terracería la hidratación baja más rápido
            if (game.estado == STATUS_JUGANDO && game.tasaAgua > 2.0f) {
                float parp = 0.65f + 0.35f * std::sin((float)glfwGetTime() * 5.0f);
                const std::string txtCalor = "¡CALOR! HIDRÁTATE: TE DESHIDRATAS MÁS RÁPIDO";
                float anchoC = Texto::Ancho(txtCalor, 2.0f);
                Texto::Panel(W - anchoC - 40.0f, 16.0f, anchoC + 28.0f, 36.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.55f), W, H);
                Texto::Dibujar(txtCalor, W - anchoC - 26.0f, 26.0f, 2.0f,
                    glm::vec4(1.0f, 0.55f, 0.2f, parp), W, H);
            
            }

            // Aviso al salirse de la pista: novato cada vez; intermedia solo la primera vez
            bool mostrarAvisoPista = (game.dificultad == DIF_BASICA && fueraPistaTimer > 0.3f)
                || (game.dificultad == DIF_INTERMEDIA && avisoPistaTimer > 0.0f);
            if (mostrarAvisoPista && game.estado == STATUS_JUGANDO && !pausado && !enMenu) {
                float parp = 0.7f + 0.3f * std::sin((float)glfwGetTime() * 6.0f);
                Texto::Panel((W - 860.0f) * 0.5f, H * 0.30f - 46.0f, 860.0f, 92.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.60f), W, H);
                centrado("¡TE SALISTE DE LA PISTA!", H * 0.30f + 10.0f, 4.0f, glm::vec4(1.0f, 0.55f, 0.2f, parp));
                centrado("REGRESA AL CAMINO: TUS LLANTAS SE DESGASTAN MÁS RÁPIDO", H * 0.30f - 28.0f, 2.2f, BLANCO);
            }

            // Aviso grande de multa
            if (avisoTimer > 0.0f && game.estado == STATUS_JUGANDO) {
                centrado(avisoTexto, H * 0.45f, 4.0f, glm::vec4(1.0f, 0.3f, 0.3f, std::min(1.0f, avisoTimer)));
            }

            // Aviso de vida perdida
            if (game.avisoVidaTimer > 0.0f && game.estado == STATUS_JUGANDO) {
                std::vector<std::string> ls = Narracion::Ajustar(Narracion::Preparar(game.avisoVidaTexto), W - 120.0f, 3.0f);
                float yv = H * 0.40f;
                for (const std::string& l : ls) { centrado(l, yv, 3.0f, glm::vec4(1.0f, 0.3f, 0.3f, std::min(1.0f, game.avisoVidaTimer))); yv -= 34.0f; }
            }

            // Recuerdo obtenido (prioridad) o ayuda contextual
            if (game.estado == STATUS_JUGANDO) {
                if (recuerdoTimer > 0.0f) {
                    float a = std::min(1.0f, std::min(recuerdoTimer, 5.5f - recuerdoTimer + 0.4f) / 0.4f);
                    cajaAviso("¡RECUERDO! " + recuerdoTitulo, recuerdoTexto, H * 0.80f, std::min(1.0f, a), glm::vec4(1.0f, 0.85f, 0.2f, 1.0f));
                }
                else if (consejoTimer > 0.0f) {
                    float a = std::min(1.0f, std::min(consejoTimer, 6.0f - consejoTimer + 0.4f) / 0.4f);
                    cajaAviso(consejoTitulo, consejoTexto, H * 0.80f, std::min(1.0f, a), glm::vec4(0.4f, 0.9f, 1.0f, 1.0f));
                }
            }

            // Minimapa en esquina superior derecha con posición de jugador, semáforos y ánforas
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

                std::vector<glm::vec2> mSemaforos, mAnforas, mRecuerdos;
                for (float sSem : Semaforo::POSICIONES) {
                    glm::vec3 q = Ruta::En(sSem).pos;
                    mSemaforos.push_back(aMapa(q.x, q.z));
                }
                for (const auto& a : game.anforas) {
                    if (a.recogida) continue;
                    glm::vec3 q = Ruta::En(a.s).pos;
                    mAnforas.push_back(aMapa(q.x, q.z));
                }
                for (const auto& r : game.recuerdos) {
                    if (r.recogido) continue;
                    glm::vec3 q = Ruta::En(r.s).pos;
                    mRecuerdos.push_back(aMapa(q.x, q.z));
                }
                Texto::Cuadros(mRecuerdos, 6.0f, glm::vec4(1.0f, 0.8f, 0.1f, 1.0f), W, H);
                Texto::Cuadros(mSemaforos, 5.0f, glm::vec4(1.0f, 0.2f, 0.2f, 1.0f), W, H);
                Texto::Cuadros(mAnforas, 4.0f, glm::vec4(0.3f, 0.9f, 1.0f, 1.0f), W, H);

                std::vector<glm::vec2> mJugador = { aMapa(player.position.x, player.position.z) };
                Texto::Cuadros(mJugador, 7.0f, AMARILLO, W, H);

                Texto::Dibujar("MAPA", MX + 30.0f, MY - 18.0f, 2.0f, BLANCO, W, H);
            }

            // Banner central temporal al entrar a una nueva zona
            if (zonaTimer > 0.0f && game.estado == STATUS_JUGANDO) {
                float a = std::min(1.0f, zonaTimer);
                centrado(nombres[z], H * 0.62f, 5.0f, glm::vec4(1.0f, 0.85f, 0.2f, a));
                centrado("BUSCA EL RECUERDO DORADO DE ESTA ZONA", H * 0.62f - 34.0f, 2.5f, glm::vec4(1.0f, 1.0f, 1.0f, a));
            }

            // Fin de Juego / Victoria con Estadísticas Completas
            if (game.estado != STATUS_JUGANDO) {
                const glm::vec4 VERDE(0.3f, 1.0f, 0.4f, 1.0f), ROJO(1.0f, 0.3f, 0.3f, 1.0f);
                if (game.estado == STATUS_VICTORIA) {
                    int puntos = 1000 + game.anforasRecogidas * 50 + (int)game.psi * 10 + (int)game.agua * 5
                        - multasSemaforo * 100 - game.clavosAtropellados * 50 - (int)game.tiempoTotal * 2
                        + game.recuerdosRecogidos * 100 + (game.vidas - 1) * 80;
                    bool bonoRapido = (game.tiempoMeta > 0.0f && game.tiempoTotal <= game.tiempoMeta);
                    if (bonoRapido) puntos += 200;                                                              // intermedia: llegar antes de la meta
                    if (game.tiempoLimite > 0.0f) puntos += (int)(game.tiempoLimite - game.tiempoTotal) * 3;     // avanzada: segundos que sobraron
                    if (puntos < 0) puntos = 0;
                    std::string calif = (puntos >= 2200) ? "EXCELENTE" : (puntos >= 1700) ? "MUY BIEN" : (puntos >= 1200) ? "BIEN" : "A MEJORAR";
                    bool pasaporteCompleto = (game.recuerdosRecogidos == (int)game.recuerdos.size());

                    // Todo se acomoda desde arriba con separaciones fijas en píxeles, para que nada se encime aunque la ventana sea chica
                    Texto::Panel(0.0f, H - 545.0f, (float)W, 535.0f, glm::vec4(0.0f, 0.0f, 0.0f, 0.78f), W, H);
                    centrado("¡LLEGASTE A TEPOZTLÁN!", H - 72.0f, 6.0f, VERDE);
                    centrado("RESUMEN DEL RECORRIDO", H - 112.0f, 3.0f, AMARILLO);

                    float x0 = W * 0.5f - 260.0f, y0 = H - 152.0f, dy = 26.0f;
                    Texto::Dibujar("TIEMPO ................ " + std::to_string((int)game.tiempoTotal) + " S", x0, y0, 2.5f, BLANCO, W, H);
                    Texto::Dibujar("BOTELLAS RECOGIDAS .... " + std::to_string(game.anforasRecogidas) + "/" + std::to_string((int)game.anforas.size()), x0, y0 - dy, 2.5f, BLANCO, W, H);
                    Texto::Dibujar("CLAVOS ATROPELLADOS ... " + std::to_string(game.clavosAtropellados), x0, y0 - 2 * dy, 2.5f, BLANCO, W, H);
                    Texto::Dibujar("MULTAS DE SEMÁFORO .... " + std::to_string(multasSemaforo), x0, y0 - 3 * dy, 2.5f, BLANCO, W, H);
                    Texto::Dibujar("LLANTAS AL LLEGAR ..... " + std::to_string((int)game.psi) + " PSI", x0, y0 - 4 * dy, 2.5f, BLANCO, W, H);
                    Texto::Dibujar("HIDRATACIÓN ........... " + std::to_string((int)game.agua) + " %", x0, y0 - 5 * dy, 2.5f, BLANCO, W, H);
                    Texto::Dibujar("RECUERDOS ............. " + std::to_string(game.recuerdosRecogidos) + "/" + std::to_string((int)game.recuerdos.size()),
                        x0, y0 - 6 * dy, 2.5f, glm::vec4(1.0f, 0.85f, 0.2f, 1.0f), W, H);
                    Texto::Dibujar(std::string("VERSIÓN ............... ") + NOMBRE_DIF[game.dificultad]
                        + (bonoRapido ? "  (BONO POR LLEGAR RÁPIDO)" : ""), x0, y0 - 7 * dy, 2.5f, BLANCO, W, H);
                    centrado(pasaporteCompleto ? "¡PASAPORTE TURÍSTICO COMPLETO!" : "TE FALTARON RECUERDOS: REINTENTA PARA COMPLETAR TU PASAPORTE",
                        H - 372.0f, 2.2f, pasaporteCompleto ? VERDE : glm::vec4(1.0f, 0.85f, 0.2f, 1.0f));

                    centrado("PUNTOS: " + std::to_string(puntos) + "   " + calif, H - 420.0f, 4.0f, AMARILLO);

                    // Botón para modo exploración
                    float btnW = 380.0f, btnH = 38.0f;
                    float btnX = (W - btnW) * 0.5f;
                    float btnY = H - 478.0f;
                    Texto::Panel(btnX, btnY, btnW, btnH, glm::vec4(0.12f, 0.50f, 0.90f, 0.90f), W, H);
                    centrado((tourAuto ? "EXPLORAR LIBREMENTE [ ENTER ]" : "MODO TURISTA [ ENTER ]"), btnY + 10.0f, 2.3f, BLANCO);

                    centrado("R: REINTENTAR     M: CAMBIAR DE VERSIÓN", H - 525.0f, 2.5f, BLANCO);
                }
                else {
                    Texto::Panel(0.0f, H * 0.35f, (float)W, H * 0.30f, glm::vec4(0.0f, 0.0f, 0.0f, 0.65f), W, H);
                    centrado(game.estado == STATUS_GAME_OVER_PONCHADURA ? "LLANTA PONCHADA"
                        : (game.estado == STATUS_GAME_OVER_TIEMPO ? "SE ACABÓ EL TIEMPO" : "DESHIDRATACIÓN"),
                        H * 0.52f, 7.0f, ROJO);
                    centrado("R: REINTENTAR     M: CAMBIAR DE VERSIÓN", H * 0.39f, 3.0f, BLANCO);
                }
            }

            // Botón de pausa (arriba al centro) y pantalla de pausa
            if (!enMenu && game.estado == STATUS_JUGANDO) {
                btnPausa[2] = 190.0f; btnPausa[3] = 34.0f;
                btnPausa[0] = 680.0f; btnPausa[1] = H - 50.0f;   // a la derecha del panel de zona
                Texto::Panel(btnPausa[0], btnPausa[1], btnPausa[2], btnPausa[3],
                    pausado ? glm::vec4(0.85f, 0.45f, 0.05f, 0.90f) : glm::vec4(0.0f, 0.0f, 0.0f, 0.55f), W, H);
                Texto::Dibujar(pausado ? "[P] CONTINUAR" : "[P] PAUSA", btnPausa[0] + 14.0f, btnPausa[1] + 10.0f, 2.0f, BLANCO, W, H);
                Texto::Dibujar(Audio::Silenciado() ? "[V] SONIDO: NO" : "[V] SONIDO: SI", btnPausa[0] + btnPausa[2] + 14.0f, btnPausa[1] + 10.0f, 2.0f,
                    Audio::Silenciado() ? glm::vec4(1.0f, 0.4f, 0.4f, 1.0f) : BLANCO, W, H);
            }
            else { btnPausa[2] = 0.0f; btnPausa[3] = 0.0f; }
            if (pausado && !enMenu) {
                Texto::Panel(0.0f, H * 0.38f, (float)W, H * 0.24f, glm::vec4(0.0f, 0.0f, 0.0f, 0.70f), W, H);
                centrado("PAUSA", H * 0.53f, 6.0f, AMARILLO);
                centrado("P O CLIC EN EL BOTÓN: CONTINUAR   |   H: AYUDA   |   ESC: SALIR", H * 0.43f, 2.0f, BLANCO);
            }

            // Párrafo ajustado al ancho dado (devuelve la y donde termina)
            auto parrafo = [&](const std::string& txt, float x, float y, float ancho, float esc, float lh, glm::vec4 col) {
                std::vector<std::string> ls = Narracion::Ajustar(Narracion::Preparar(txt), ancho, esc);
                for (const std::string& l : ls) { Texto::Dibujar(l, x, y, esc, col, W, H); y -= lh; }
                return y;
                };
            float parpadeo = 0.55f + 0.45f * std::sin((float)glfwGetTime() * 4.0f);
            const glm::vec4 VERDE_P(0.3f, 1.0f, 0.4f, parpadeo);

            if (enBienvenida) {
                // Ventana de bienvenida: qué es el juego
                Texto::Panel(0.0f, 0.0f, (float)W, (float)H, glm::vec4(0.0f, 0.0f, 0.0f, 0.85f), W, H);
                float pw = std::min(900.0f, W - 60.0f), px = (W - pw) * 0.5f;
                Texto::Panel(px - 20.0f, H * 0.14f, pw + 40.0f, H * 0.76f, glm::vec4(0.10f, 0.25f, 0.45f, 0.80f), W, H);
                centrado("¡BIENVENIDO!", H * 0.82f, 6.0f, AMARILLO);
                centrado("CICLOVÍA FERROCARRIL DE CUERNAVACA", H * 0.74f, 3.5f, BLANCO);
                float y = H * 0.66f;
                y = parrafo("Este es un juego de cicloturismo: recorrerás en bicicleta la ruta que va de Polanco, en la Ciudad de México, hasta el pueblo de Tepoztlán, en Morelos, siguiendo la huella del antiguo Ferrocarril de Cuernavaca.",
                    px, y, pw, 2.2f, 28.0f, BLANCO) - 10.0f;
                y = parrafo("Pasarás por cuatro paisajes: la ciudad, un bosque de ocotes, un bioma árido con ganadería y una terracería. Cuida el aire de tus llantas y tu hidratación, respeta los semáforos y evita los clavos.",
                    px, y, pw, 2.2f, 28.0f, BLANCO) - 10.0f;
                y = parrafo("Como buen turista, en cada zona busca el recuerdo dorado: con ellos armas tu pasaporte turístico y conoces la historia y la cultura del camino.",
                    px, y, pw, 2.2f, 28.0f, glm::vec4(1.0f, 0.85f, 0.2f, 1.0f)) - 10.0f;
                parrafo("A continuación elegirás tu versión: novato, intermedia o avanzada, según tu experiencia con juegos y aplicaciones 3D.",
                    px, y, pw, 2.2f, 28.0f, BLANCO);
                centrado("PRESIONA ENTER PARA CONTINUAR", H * 0.07f, 3.5f, VERDE_P);
            }
            else if (enMenu && !partidaIniciada) {
                // Selección de versión según la experiencia del usuario
                Texto::Panel(0.0f, 0.0f, (float)W, (float)H, glm::vec4(0.0f, 0.0f, 0.0f, 0.85f), W, H);
                centrado("ELIGE TU VERSIÓN", H * 0.90f, 5.0f, AMARILLO);
                centrado("SEGÚN TU EXPERIENCIA CON JUEGOS Y APLICACIONES 3D", H * 0.84f, 2.2f, BLANCO);

                const char* sub[3] = { "PARA PRINCIPIANTES", "CON ALGO DE EXPERIENCIA", "PARA EXPERTOS" };
                const char* desc[3] = {
                    "3 vidas. En cada botella, semáforo, clavo y bomba de aire te recordamos qué hacer. Sin tiempo límite.",
                    "1 vida. Las ayudas solo aparecen la primera vez de cada cosa. Sin tiempo límite, pero ganas un bono si llegas en menos de 3:20.",
                    "1 vida y sin ayudas. Tienes 2:50 para terminar el recorrido y gastas más agua y aire. Los clavos y las multas pegan más fuerte." };
                float cw = (W - 80.0f - 40.0f) / 3.0f, ch = 250.0f, top = H * 0.78f;
                for (int i = 0; i < 3; i++) {
                    float cx = 40.0f + i * (cw + 20.0f);
                    bool sel = (i == seleccionDif);
                    if (sel) Texto::Panel(cx - 6.0f, top - ch - 6.0f, cw + 12.0f, ch + 12.0f, glm::vec4(0.3f, 1.0f, 0.4f, 0.95f), W, H);
                    Texto::Panel(cx, top - ch, cw, ch, sel ? glm::vec4(0.10f, 0.25f, 0.45f, 1.0f) : glm::vec4(0.12f, 0.12f, 0.14f, 1.0f), W, H);
                    Texto::Dibujar(std::to_string(i + 1) + "  " + NOMBRE_DIF[i], cx + 14.0f, top - 38.0f, 3.0f, sel ? AMARILLO : BLANCO, W, H);
                    Texto::Dibujar(sub[i], cx + 14.0f, top - 66.0f, 1.8f, glm::vec4(0.6f, 0.85f, 1.0f, 1.0f), W, H);
                    parrafo(desc[i], cx + 14.0f, top - 100.0f, cw - 28.0f, 2.0f, 24.0f, BLANCO);
                }

                // Controles (dos columnas)
                const char* col1[] = { "W / FLECHA ARRIBA .. PEDALEAR", "S / FLECHA ABAJO ... FRENAR", "A D / FLECHAS ..... GIRAR",
                    "SHIFT ............. IMPULSO", "C ................. CAMBIAR CÁMARA", "P ................. PAUSA", "H ................. AYUDA",
                    "V ................. SONIDO ON/OFF" };
                const char* col2[] = { "F ................. FARO ON/OFF", "N ................. DÍA / NOCHE", "R ................. REINTENTAR",
                    "M ................. CAMBIAR VERSIÓN", "T ................. VERSIÓN DEMO (SOLO AQUÍ)", "TAB ............... LIBERAR RATÓN", "ESC ............... SALIR",
                    " " };
                float yc = H * 0.78f - ch - 28.0f;
                Texto::Dibujar("CONTROLES", W * 0.5f - Texto::Ancho("CONTROLES", 2.5f) * 0.5f, yc + 6.0f, 2.5f, AMARILLO, W, H);
                yc -= 24.0f;
                for (int i = 0; i < 8; i++) {
                    Texto::Dibujar(col1[i], 60.0f, yc, 2.0f, BLANCO, W, H);
                    Texto::Dibujar(col2[i], W * 0.5f + 20.0f, yc, 2.0f, BLANCO, W, H);
                    yc -= 22.0f;
                }
                centrado("1 2 3 O FLECHAS: ELEGIR VERSIÓN", H * 0.115f, 2.2f, glm::vec4(0.6f, 0.85f, 1.0f, 1.0f));
                centrado("T: VERSIÓN DEMO (RECORRIDO AUTOMÁTICO, AL TERMINAR PODRÁS EXPLORAR LIBREMENTE)", H * 0.075f, 2.0f, glm::vec4(1.0f, 0.85f, 0.2f, 1.0f));
                centrado(std::string("ENTER: COMENZAR EN VERSIÓN ") + NOMBRE_DIF[seleccionDif], H * 0.045f, 3.0f, VERDE_P);
            }
            else if (enMenu) {
                // Ayuda con los controles durante la partida (tecla H)
                Texto::Panel(0.0f, 0.0f, (float)W, (float)H, glm::vec4(0.0f, 0.0f, 0.0f, 0.78f), W, H);
                centrado("CICLOVÍA FERROCARRIL DE CUERNAVACA", H * 0.82f, 5.0f, AMARILLO);
                centrado(std::string("VERSIÓN ") + NOMBRE_DIF[game.dificultad] + "   |   CICLOTURISMO CDMX - TEPOZTLÁN", H * 0.75f, 2.5f, BLANCO);

                std::vector<std::string> controles = {
                    "W / FLECHA ARRIBA ... PEDALEAR",
                    "S / FLECHA ABAJO .... FRENAR",
                    "A D / FLECHAS ....... GIRAR",
                    "SHIFT ............... IMPULSO",
                    "C ................... CAMBIAR CÁMARA",
                    "F ................... FARO ON/OFF",
                    "N ................... DÍA / NOCHE",
                    "R ................... REINTENTAR",
                    "M ................... CAMBIAR DE VERSIÓN",
                    "P ................... PAUSA / CONTINUAR",
                    "H ................... AYUDA",
                    "V ................... SONIDO ON/OFF",
                    "TAB ................. LIBERAR / CAPTURAR RATON",
                    "ESC ................. SALIR" };
                float yc = H * 0.70f;
                for (const std::string& linea : controles) {
                    Texto::Dibujar(linea, W * 0.5f - 270.0f, yc, 2.3f, BLANCO, W, H);
                    yc -= 21.0f;
                }

                centrado("OBJETIVO: LLEGA A TEPOZTLÁN Y RECOGE UN RECUERDO EN CADA ZONA", H * 0.25f, 2.5f, AMARILLO);
                centrado("RECOGE BOTELLAS DE AGUA, ESQUIVA LOS CLAVOS E INFLA EN LAS BOMBAS DE AIRE", H * 0.20f, 2.0f, BLANCO);
                centrado("PRESIONA H O ENTER PARA CONTINUAR", H * 0.10f, 3.5f, VERDE_P);
            }
        }

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
    }

    // Liberación de recursos de interfaz y cierre de GLFW
    if (hudVAO) glDeleteVertexArrays(1, &hudVAO);
    if (hudVBO) glDeleteBuffers(1, &hudVBO);
    if (hudProgram) glDeleteProgram(hudProgram);
    Texto::Fin();
    Audio::Fin();

    glfwTerminate();
    return 0;
}

// Actualiza posición sobre la ruta, lógica de juego, infracciones y movimiento del jugador
void DoMovement() {
    if (enBienvenida || enMenu || pausado) return;
    static glm::vec3 prevPos = player.position;
    float velocidadReal = glm::length(player.position - prevPos) / (deltaTime > 0.0001f ? deltaTime : 0.016f);
    prevPos = player.position;

    // Proyección de la posición mundial (X, Z) sobre las muestras de la ruta para obtener distancia (s) y desviación lateral
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

    // Actualización de físicas, consumo y colisiones en GameLogic
    PolancoScene::riderS = sActual;
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

    // Novato: avisa cada vez que el ciclista se sale de la pista. Intermedia: solo la primera vez. Avanzada: nunca.
    // (mismo límite que usa GameLogic para desgastar las llantas)
    if ((game.dificultad == DIF_BASICA || game.dificultad == DIF_INTERMEDIA) && game.estado == STATUS_JUGANDO
        && !tourAuto && !modoExploracionTepoz && sActual < 475.0f && std::abs(lateralActual) > 1.35f)
        fueraPistaTimer += deltaTime;
    else
        fueraPistaTimer = 0.0f;
    if (game.dificultad == DIF_INTERMEDIA && !vistoAvisoPista && fueraPistaTimer > 0.3f) {
        vistoAvisoPista = true;
        avisoPistaTimer = 5.0f;
    }
    if (avisoPistaTimer > 0.0f) avisoPistaTimer -= deltaTime;

    // Multas de semáforo: penaliza -5 PSI si cruza la línea cuando la fase está en rojo
    static float sAnterior = 0.0f;
    if (game.estado == STATUS_JUGANDO && !modoExploracionTepoz) {
        for (float sSem : Semaforo::POSICIONES) {
            if (sAnterior < sSem && sActual >= sSem && sActual - sAnterior < 20.0f
                && Semaforo::FaseEn((float)glfwGetTime()) == Semaforo::ROJO) {
                game.psi = std::max(0.0f, game.psi - game.psiPorMulta);
                multasSemaforo++;
                avisoTexto = "¡CRUZASTE EN ROJO! -" + std::to_string((int)game.psiPorMulta) + " PSI";
                avisoTimer = 3.0f;
            }
        }
    }
    sAnterior = sActual;
    if (avisoTimer > 0.0f) avisoTimer -= deltaTime;
    if (consejoTimer > 0.0f) consejoTimer -= deltaTime;
    if (recuerdoTimer > 0.0f) recuerdoTimer -= deltaTime;

    // Ayudas contextuales según la versión (ver MostrarConsejo) y avisos de recuerdos turísticos
    if (game.estado == STATUS_JUGANDO && !modoExploracionTepoz) {
        if (game.evRecuerdo >= 0) {
            const Recuerdo& r = game.recuerdos[game.evRecuerdo];
            recuerdoTitulo = r.nombre;
            recuerdoTexto = r.dato;
            recuerdoTimer = 5.5f;
        }
        if (game.evBotella)
            MostrarConsejo(0, "BOTELLA RECOGIDA", "Recuperaste " + std::to_string((int)game.aguaPorBotella)
                + " % de hidratación. Recoge más botellas: en el bioma árido y la terracería te deshidratas más rápido.");
        if (game.evClavo)
            MostrarConsejo(3, "¡CLAVO!", "Perdiste " + std::to_string((int)game.psiPorClavo)
                + " PSI. Busca una bomba de aire y frena casi por completo junto a ella para inflar tus llantas.");
        if (game.evBombaInicio)
            MostrarConsejo(2, "INFLANDO LLANTAS", "¡Muy bien! Mantente junto a la bomba hasta que la barra de PSI se llene.");

        // Avisos al acercarse (una vez por objeto): botella, semáforo, bomba y clavos
        for (size_t i = 0; i < game.anforas.size() && i < 16; i++) {
            const auto& a = game.anforas[i];
            if (!a.recogida && !avisoBotella[i] && sActual > a.s - 18.0f && sActual < a.s - 3.0f) {
                avisoBotella[i] = true;
                MostrarConsejo(0, "BOTELLA DE AGUA ADELANTE", "Pasa sobre ella para recuperar hidratación. Si llegas a 0 %, pierdes una vida.");
            }
        }
        for (int i = 0; i < 2; i++) {
            float sSem = Semaforo::POSICIONES[i];
            if (!avisoSem[i] && sActual > sSem - 30.0f && sActual < sSem - 4.0f) {
                avisoSem[i] = true;
                MostrarConsejo(1, "SEMÁFORO ADELANTE", "Si está en rojo, frena con S y espera el verde. Cruzar en rojo te quita "
                    + std::to_string((int)game.psiPorMulta) + " PSI.");
            }
        }
        for (size_t i = 0; i < game.talleres.size() && i < 8; i++) {
            const auto& t = game.talleres[i];
            if (!avisoBomba[i] && sActual > t.s - 25.0f && sActual < t.s - 3.0f) {
                avisoBomba[i] = true;
                if (game.psi < game.PSI_MAX - 4.0f)
                    MostrarConsejo(2, "BOMBA DE AIRE ADELANTE", "Tus llantas están bajas: frena casi por completo junto a la bomba para inflarlas.");
            }
        }
        for (size_t i = 0; i < game.clavos.size() && i < 16; i++) {
            const auto& c = game.clavos[i];
            if (!c.activado && !avisoClavo[i] && sActual > c.s - 14.0f && sActual < c.s - 2.0f) {
                avisoClavo[i] = true;
                MostrarConsejo(3, "CUIDADO: CLAVOS", "Hay clavos en el camino. Esquívalos moviéndote a un lado con A o D.");
            }
        }
    }

    // Detección de cambio de zona geográfica para disparar el banner informativo
    int zonaNueva = (sActual < 210.0f) ? 0 : (sActual < 320.0f) ? 1 : (sActual < 415.0f) ? 2 : (sActual < 475.0f) ? 3 : 4;
    if (zonaNueva != zonaActual) { zonaActual = zonaNueva; zonaTimer = 4.0f; }
    if (zonaTimer > 0.0f) zonaTimer -= deltaTime;

    // Cajas de indicaciones y datos interesantes por zona (ver Narracion.h)
    if (game.estado == STATUS_JUGANDO) Narracion::Actualizar(sActual, deltaTime);

    // Notificaciones en consola al cambiar el estado de la partida
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
        else if (game.estado == STATUS_GAME_OVER_TIEMPO) {
            std::cout << "\n[GAME OVER] Se acabo el tiempo. Presiona 'R' para reiniciar." << std::endl;
        }
    }

    // Controles y Piloto Automático (simula entradas de teclado siguiendo la curva y respetando semáforos/talleres)
    if (game.estado == STATUS_GAME_OVER_PONCHADURA || game.estado == STATUS_GAME_OVER_DESHIDRATACION
        || game.estado == STATUS_GAME_OVER_TIEMPO) {
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

    // Desplazamiento de la cámara libre con teclas I/J/K/L
    if (cameraMode == 2) {
        if (keys[GLFW_KEY_I]) camera.ProcessKeyboard(FORWARD, deltaTime);
        if (keys[GLFW_KEY_K]) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (keys[GLFW_KEY_J]) camera.ProcessKeyboard(LEFT, deltaTime);
        if (keys[GLFW_KEY_L]) camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

// Gestiona las pulsaciones de teclado para acciones puntuales (cámaras, menú, faro, día/noche y reinicio)
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
        cameraMode = (cameraMode + 1) % 4;
        // Cámara libre: el ratón se captura para mirar. En las demás cámaras el ratón queda libre para usar la ventana.
        glfwSetInputMode(window, GLFW_CURSOR, cameraMode == 2 ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        firstMouse = true;
    }

    // Selección de versión (solo en el menú de inicio): 1, 2, 3 o flechas izquierda / derecha
    if (enMenu && !enBienvenida && !partidaIniciada && action == GLFW_PRESS) {
        if (key == GLFW_KEY_1) seleccionDif = 0;
        if (key == GLFW_KEY_2) seleccionDif = 1;
        if (key == GLFW_KEY_3) seleccionDif = 2;
        if (key == GLFW_KEY_LEFT)  seleccionDif = std::max(0, seleccionDif - 1);
        if (key == GLFW_KEY_RIGHT) seleccionDif = std::min(2, seleccionDif + 1);
    }

    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        if (enBienvenida) {
            enBienvenida = false;                    // de la bienvenida al menú de versiones
        }
        else if (enMenu && !partidaIniciada) {
            game.dificultad = (Dificultad)seleccionDif;   // empieza la partida con la versión elegida
            partidaIniciada = true;
            ResetToStart();
            enMenu = false;
            zonaActual = -1;
        }
        else if (enMenu) {
            enMenu = false;
            zonaActual = -1;
        }
        else if (game.estado == STATUS_VICTORIA) {
            game.estado = STATUS_JUGANDO;
            modoExploracionTepoz = true;
            if (tourAuto) { tourAuto = false; cameraMode = 1; }   // fin de la demo: ya puedes moverte libremente
        }
    }

    // TAB: libera o captura el ratón en cualquier momento (por si quedó atrapado en la ventana)
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        bool libre = (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL);
        glfwSetInputMode(window, GLFW_CURSOR, libre ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        firstMouse = true;
    }

    // P: pausa / continuar (solo durante la partida)
    if (key == GLFW_KEY_P && action == GLFW_PRESS && !enMenu && game.estado == STATUS_JUGANDO) {
        pausado = !pausado;
    }

    if (key == GLFW_KEY_H && action == GLFW_PRESS && !enBienvenida && partidaIniciada) {
        enMenu = !enMenu;     // ayuda con los controles durante la partida
    }

    // M: volver al menú para elegir otra versión
    if (key == GLFW_KEY_M && action == GLFW_PRESS && !enBienvenida) {
        partidaIniciada = false;
        enMenu = true;
        tourAuto = false;
        modoExploracionTepoz = false;
        ResetToStart();
    }

    // T: versión demo, solo desde el menú de selección de versión
    if (key == GLFW_KEY_T && action == GLFW_PRESS && enMenu && !enBienvenida && !partidaIniciada) {
        game.dificultad = DIF_BASICA;
        partidaIniciada = true;
        ResetToStart();
        tourAuto = true;
        tourT = 0.0f;
        enMenu = false;
        zonaActual = -1;
    }

    if (key == GLFW_KEY_R && action == GLFW_PRESS && !enBienvenida && !enMenu) {
        ResetToStart();
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS) { faroEncendido = !faroEncendido; }
    if (key == GLFW_KEY_N && action == GLFW_PRESS) { esDeNoche = !esDeNoche; }
    if (key == GLFW_KEY_V && action == GLFW_PRESS) { Audio::AlternarSilencio(); }   // sonido ON/OFF
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        PrintPosition();
    }
}

// Restaura al jugador, la cámara y las variables de partida al inicio de la ruta
void ResetToStart() {
    pausado = false;
    player.position = startPosition;
    player.rotation = startRotation;

    game.Reiniciar();
    Narracion::MostrarIndicaciones() = (game.dificultad != DIF_AVANZADA);   // la versión avanzada no tiene indicaciones
    Narracion::Reiniciar();
    std::fill(avisoBotella, avisoBotella + 16, false);
    std::fill(avisoSem, avisoSem + 4, false);
    std::fill(avisoBomba, avisoBomba + 8, false);
    std::fill(avisoClavo, avisoClavo + 16, false);
    consejoTimer = 0.0f;
    recuerdoTimer = 0.0f;
    for (int i = 0; i < 4; i++) vistoAyuda[i] = false;
    zonaActual = -1;
    multasSemaforo = 0;
    avisoTimer = 0.0f;
    fueraPistaTimer = 0.0f;
    avisoPistaTimer = 0.0f;
    vistoAvisoPista = false;
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

// Imprime en consola las coordenadas actuales y métricas del jugador
void PrintPosition() {
    std::cout << "[Telemetria] Posicion=(" << player.position.x << ", "
        << player.position.y << ", " << player.position.z << ") | Presion="
        << game.psi << " PSI | Agua=" << game.agua << "%" << std::endl;
}

// Procesa el movimiento del ratón para orientar la cámara en modo libre
// Clic izquierdo sobre el botón de pausa
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) return;
    if (enMenu || btnPausa[2] <= 0.0f) return;
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) return;
    double mx, my; int ww, wh, fw, fh;
    glfwGetCursorPos(window, &mx, &my);
    glfwGetWindowSize(window, &ww, &wh);
    glfwGetFramebufferSize(window, &fw, &fh);
    float x = (float)mx * fw / (float)ww;
    float y = fh - (float)my * fh / (float)wh;   // origen abajo-izquierda, igual que Texto
    if (x >= btnPausa[0] && x <= btnPausa[0] + btnPausa[2] && y >= btnPausa[1] && y <= btnPausa[1] + btnPausa[3])
        pausado = !pausado;
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

// Inicializa el quad 2D y los shaders ortográficos para dibujar las barras de estado
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

// Dibuja en pantalla las barras de progreso de presión PSI e hidratación mediante proyección ortográfica
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

    // Barra de presión de neumáticos (cambia de color según el nivel crítico)
    drawBarra(MARGEN_X - 2.0f, 74.0f - 2.0f, ANCHO_TOTAL + 4.0f, ALTO_BARRA + 4.0f, glm::vec4(0.1f, 0.1f, 0.1f, 0.8f));
    float pctPsi = std::max(0.0f, std::min(1.0f, psi / maxPsi));
    glm::vec4 colPsi = (pctPsi > 0.45f) ? glm::vec4(0.2f, 0.85f, 0.25f, 1.0f) : (pctPsi > 0.20f ? glm::vec4(0.95f, 0.75f, 0.1f, 1.0f) : glm::vec4(0.9f, 0.15f, 0.15f, 1.0f));
    drawBarra(MARGEN_X, 74.0f, ANCHO_TOTAL * pctPsi, ALTO_BARRA, colPsi);

    // Barra de nivel de hidratación
    drawBarra(MARGEN_X - 2.0f, 26.0f - 2.0f, ANCHO_TOTAL + 4.0f, ALTO_BARRA + 4.0f, glm::vec4(0.1f, 0.1f, 0.1f, 0.8f));
    float pctAgua = std::max(0.0f, std::min(1.0f, agua / maxAgua));
    drawBarra(MARGEN_X, 26.0f, ANCHO_TOTAL * pctAgua, ALTO_BARRA, glm::vec4(0.15f, 0.60f, 0.95f, 1.0f));

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}