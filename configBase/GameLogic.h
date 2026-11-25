// Reglas del juego: llantas, hidratación, objetos del camino y puntaje.
// El ciclista recorre la ruta de Polanco a Tepoztlán y se le va gastando el aire
// de las llantas y el agua. Esta clase decide qué pasa en cada momento y dibuja
// los objetos del juego (botellas, clavos y bombas de aire).
#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <string>
#include "Ruta.h"
#include "Shader.h"
#include "Model.h"

// Estados posibles de la partida.
enum GameStatus {
    // La partida sigue en curso
    STATUS_JUGANDO,
    // Perdió: se quedó sin aire en las llantas
    STATUS_GAME_OVER_PONCHADURA,
    // Perdió: se quedó sin agua
    STATUS_GAME_OVER_DESHIDRATACION,
    // Ganó: llegó a Tepoztlán
    STATUS_VICTORIA,
    // Perdió: se acabó el tiempo (solo en la versión avanzada)
    STATUS_GAME_OVER_TIEMPO
};

// Versión del juego según la experiencia del usuario.
enum Dificultad {
    DIF_BASICA,        // 3 vidas, ayudas en cada botella, semáforo, clavo y bomba
    DIF_INTERMEDIA,    // 1 vida, ayudas solo la primera vez, bono por llegar rápido
    DIF_AVANZADA       // 1 vida, sin ayudas y con tiempo límite
};

// Recuerdo turístico: un objeto dorado en cada zona que simula "llevarse un souvenir" del lugar.
struct Recuerdo {
    float s;
    float lateral;
    const char* nombre;
    const char* dato;
    bool recogido;
};

// En los objetos del camino, "s" es la distancia recorrida sobre la ruta (en metros
// desde la salida) y "lateral" es qué tan a la izquierda o derecha del centro está.

// Botella de agua que el ciclista recoge para hidratarse.
struct AnforaAgua {
    float s;
    float lateral;
    // Ya fue recogida (deja de dibujarse)
    bool recogida;
};

// Clavos en el piso que le sacan aire a la llanta.
struct PeligroClavos {
    float s;
    float lateral;
    // Ya se pisaron (dejan de dibujarse)
    bool activado;
};

// Bomba de aire a la orilla del camino para volver a inflar las llantas.
struct TallerInflado {
    float s;
    float lateral;
    bool activo;
};

// Controla toda la lógica de la partida.
class GameLogic {
public:
    // Aire actual de las llantas (35 es el máximo)
    float psi;
    const float PSI_MAX = 35.0f;
    // Hidratación actual (100 es el máximo)
    float agua;
    const float AGUA_MAX = 100.0f;
    // Cuánta agua se pierde por segundo según el bioma donde va el ciclista (sin contar la velocidad).
    // main.cpp la usa para avisar en pantalla cuando hace calor.
    float tasaAgua;
    // Botellas recogidas (suman puntos)
    int anforasRecogidas;
    // Clavos pisados (restan puntos)
    int clavosAtropellados;
    // Segundos transcurridos en la partida
    float tiempoTotal;
    // Tiempos de las versiones (segundos)
    const float TIEMPO_LIMITE_AVANZADA = 170.0f;
    const float TIEMPO_META_INTERMEDIA = 200.0f;
    // Si se sigue jugando, se ganó o se perdió
    GameStatus estado;

    // ---- Versión del juego (se elige en el menú) ----
    Dificultad dificultad;
    int vidas;                      // vidas que quedan
    int vidasMax;                   // vidas con las que empezó
    float tiempoLimite;             // segundos para terminar (solo avanzada; 0 = sin límite)
    float tiempoMeta;               // segundos para el bono por llegar rápido (solo intermedia)
    float aguaPorBotella;           // hidratación que recupera cada botella
    float psiPorClavo;              // aire que quita cada clavo
    float psiPorMulta;              // aire que quita cruzar en rojo
    float factorAgua;               // multiplica el consumo de agua
    float avisoVidaTimer;           // segundos que se muestra "perdiste una vida"
    std::string avisoVidaTexto;

    // Recuerdos turísticos repartidos por las zonas
    std::vector<Recuerdo> recuerdos;
    int recuerdosRecogidos;

    // Eventos del último cuadro (main.cpp los lee para mostrar las ayudas)
    bool evBotella, evClavo, evBombaInicio, evVidaPerdida;
    int evRecuerdo;                 // índice del recuerdo recogido en este cuadro (-1 si ninguno)

    // Objetos repartidos por la ruta
    std::vector<AnforaAgua> anforas;
    std::vector<PeligroClavos> clavos;
    std::vector<TallerInflado> talleres;

    // Tiempo que dura el aviso en pantalla al pisar un clavo
    float alertaDanioTimer;
    // Indica si en este momento se está inflando la llanta
    bool inflandoActualmente;

    // Punteros a los modelos 3D
    Model* botellaModel;
    Model* clavosModel;
    Model* bombaModel;
    // Figura dorada de los recuerdos (se genera en código)
    GLuint recVAO = 0, recVBO = 0;
    int recVerts = 0;

    // Al crear el juego se dejan los modelos vacíos y se arma una partida nueva.
    GameLogic() {
        botellaModel = nullptr;
        clavosModel = nullptr;
        bombaModel = nullptr;
        dificultad = DIF_INTERMEDIA;
        Reiniciar();
    }

    // Libera los modelos de memoria al cerrar el programa.
    ~GameLogic() {
        delete botellaModel;
        delete clavosModel;
        delete bombaModel;
        if (recVAO) glDeleteVertexArrays(1, &recVAO);
        if (recVBO) glDeleteBuffers(1, &recVBO);
    }

    // Carga de modelos (debe ejecutarse en main DESPUÉS de glewInit)
    void Init() {
        botellaModel = new Model((char*)"../assets/obj/botella/botella.obj");
        clavosModel = new Model((char*)"../assets/obj/Clavos/clavos.obj");
        bombaModel = new Model((char*)"../assets/obj/bomba-aire/bomba-aire.obj");

        // Figura de los recuerdos: un diamante (octaedro alargado) con normales por cara
        {
            const glm::vec3 top(0.0f, 1.0f, 0.0f), bot(0.0f, -0.6f, 0.0f);
            const glm::vec3 ring[4] = { glm::vec3(0.45f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.45f),
                                        glm::vec3(-0.45f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -0.45f) };
            std::vector<float> v;
            auto tri = [&](glm::vec3 a, glm::vec3 b, glm::vec3 c) {
                glm::vec3 n = glm::normalize(glm::cross(b - a, c - a));
                glm::vec3 p[3] = { a, b, c };
                for (int i = 0; i < 3; i++) {
                    float f[8] = { p[i].x, p[i].y, p[i].z, n.x, n.y, n.z, 0.0f, 0.0f };
                    v.insert(v.end(), f, f + 8);
                }
                };
            for (int i = 0; i < 4; i++) {
                int j = (i + 1) % 4;
                tri(top, ring[j], ring[i]);
                tri(bot, ring[i], ring[j]);
            }
            recVerts = (int)(v.size() / 8);
            glGenVertexArrays(1, &recVAO);
            glGenBuffers(1, &recVBO);
            glBindVertexArray(recVAO);
            glBindBuffer(GL_ARRAY_BUFFER, recVBO);
            glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);
            glBindVertexArray(0);
        }

        std::cout << "\n========================================" << std::endl;
        std::cout << "[GameLogic] Carga de modelos completada." << std::endl;
        std::cout << "========================================\n" << std::endl;
    }

    // Deja todo como al inicio: llantas y agua llenas, contadores en cero
    // y todos los objetos del camino colocados de nuevo en su lugar.
    void Reiniciar() {
        psi = PSI_MAX;
        agua = AGUA_MAX;
        tasaAgua = 1.2f;
        anforasRecogidas = 0;
        clavosAtropellados = 0;
        tiempoTotal = 0.0f;
        alertaDanioTimer = 0.0f;
        inflandoActualmente = false;
        estado = STATUS_JUGANDO;

        // Reglas según la versión elegida
        evBotella = evClavo = evBombaInicio = evVidaPerdida = false;
        evRecuerdo = -1;
        avisoVidaTimer = 0.0f;
        avisoVidaTexto.clear();
        switch (dificultad) {
        case DIF_BASICA:
            vidasMax = 3; tiempoLimite = 0.0f; tiempoMeta = 0.0f;
            aguaPorBotella = 30.0f; psiPorClavo = 8.0f; psiPorMulta = 3.0f; factorAgua = 0.85f;
            break;
        case DIF_AVANZADA:
            vidasMax = 1; tiempoLimite = TIEMPO_LIMITE_AVANZADA; tiempoMeta = 0.0f;
            aguaPorBotella = 20.0f; psiPorClavo = 15.0f; psiPorMulta = 8.0f; factorAgua = 1.15f;
            break;
        default: // DIF_INTERMEDIA
            vidasMax = 1; tiempoLimite = 0.0f; tiempoMeta = TIEMPO_META_INTERMEDIA;
            aguaPorBotella = 25.0f; psiPorClavo = 12.0f; psiPorMulta = 5.0f; factorAgua = 1.0f;
            break;
        }
        vidas = vidasMax;

        // Recuerdos turísticos: uno o dos por zona (zona 1 Polanco, 2 bosque, 3 árido, 4 terracería, 5 Tepoztlán)
        recuerdos.clear();
        recuerdosRecogidos = 0;
        recuerdos.push_back({  50.0f, 0.0f, "FOTO DEL MUSEO SOUMAYA",
            "Su fachada tiene 16 mil hexágonos de aluminio.", false });
        recuerdos.push_back({ 116.0f, 0.0f, "BOLETO DEL FERROCARRIL",
            "Esta ciclovía sigue casi la ruta del antiguo Ferrocarril de Cuernavaca.", false });
        recuerdos.push_back({ 228.0f, 0.0f, "POSTAL DE EMILIANO ZAPATA",
            "Líder campesino de Morelos, con el lema Tierra y Libertad.", false });
        recuerdos.push_back({ 288.0f, 0.0f, "FOTO EN EL PUENTE ROJO",
            "Un gran spot para fotos en la Ciclovía Rural.", false });
        recuerdos.push_back({ 372.0f, 0.0f, "BORREGO DE BARRO",
            "En esta zona de ganadería se cría el borrego para la barbacoa.", false });
        recuerdos.push_back({ 448.0f, 0.0f, "FOTO DEL CERRO DEL TEPOZTECO",
            "En lo alto hay un templo prehispánico dedicado a Tepoztécatl.", false });
        recuerdos.push_back({ 476.0f, 0.0f, "ARTESANÍA DE TEPOZTLÁN",
            "Llévate un recuerdo de este Pueblo Mágico.", false });

        anforas.clear();
        clavos.clear();
        talleres.clear();

        // 1. Botellas de agua (en el bioma árido hay más, porque ahí te deshidratas más rápido)
        anforas.push_back({ 25.0f,   0.3f, false });
        anforas.push_back({ 75.0f,  -0.4f, false });
        anforas.push_back({ 150.0f,  0.4f, false });
        anforas.push_back({ 240.0f, -0.3f, false });
        anforas.push_back({ 310.0f,  0.3f, false });
        anforas.push_back({ 345.0f, -0.3f, false });
        anforas.push_back({ 365.0f, -0.4f, false });
        anforas.push_back({ 392.0f,  0.3f, false });
        anforas.push_back({ 425.0f,  0.2f, false });

        // 2. Clavos
        clavos.push_back({ 55.0f,   0.2f, false });
        clavos.push_back({ 120.0f, -0.3f, false });
        clavos.push_back({ 270.0f,  0.4f, false });
        clavos.push_back({ 340.0f, -0.2f, false });
        clavos.push_back({ 400.0f,  0.3f, false });

        // 3. Talleres de inflado
        talleres.push_back({ 90.0f,   1.7f, true });
        talleres.push_back({ 200.0f, -1.7f, true });
        talleres.push_back({ 350.0f,  1.8f, true });
        talleres.push_back({ 415.0f, -1.8f, true });
    }

    // Avanza el juego un instante (se llama en cada cuadro).
    // Recibe el tiempo transcurrido, dónde va el ciclista y qué tan rápido pedalea.
    // Regresa un valor de 0 a 1 que indica qué tan bien rueda la bici: 1 con llantas
    // bien infladas y menos si están bajas (0 si la partida ya terminó).
    float Actualizar(float dt, float sJugador, float lateralJugador, float velocidadBici = 0.0f, float pendienteRuta = 0.0f) {
        // Si la partida ya terminó, no hay nada que calcular
        if (estado != STATUS_JUGANDO) return 0.0f;

        bool inflandoAntes = inflandoActualmente;
        tiempoTotal += dt;
        if (alertaDanioTimer > 0.0f) alertaDanioTimer -= dt;
        if (avisoVidaTimer > 0.0f) avisoVidaTimer -= dt;
        inflandoActualmente = false;
        evBotella = evClavo = evBombaInicio = evVidaPerdida = false;
        evRecuerdo = -1;

        // Desgaste de las llantas: salirse de la pista las desgasta más,
        // la terracería también y la calle casi nada.
        bool fueraDePista = std::abs(lateralJugador) > 1.35f;
        bool enTerraceria = (sJugador >= 415.0f);

        if (fueraDePista) {
            psi -= 3.5f * dt;
        }
        else if (enTerraceria) {
            psi -= 0.8f * dt;
        }
        else {
            psi -= 0.15f * dt;
        }

        // Consumo de agua: depende del bioma (el calor del árido y la terracería deshidrata más rápido que
        // el bosque fresco) y además es mayor mientras más veloz va el ciclista.
        if      (sJugador < 210.0f) tasaAgua = 1.2f;   // ciudad (Polanco)
        else if (sJugador < 320.0f) tasaAgua = 0.8f;   // bosque de ocotes: fresco y húmedo
        else if (sJugador < 415.0f) tasaAgua = 2.3f;   // bioma árido: calor
        else if (sJugador < 475.0f) tasaAgua = 2.6f;   // terracería: calor y polvo
        else                        tasaAgua = 1.2f;   // pueblo de Tepoztlán
        agua -= (tasaAgua + (velocidadBici * 0.06f)) * factorAgua * dt;

        // Recoger botellas: si el ciclista pasa muy cerca de una, la recoge y recupera agua.
        for (auto& anf : anforas) {
            if (!anf.recogida) {
                float ds = std::abs(anf.s - sJugador);
                float dl = std::abs(anf.lateral - lateralJugador);
                if (ds < 1.3f && dl < 0.8f) {
                    anf.recogida = true;
                    agua = std::min(AGUA_MAX, agua + aguaPorBotella);
                    anforasRecogidas++;
                    evBotella = true;
                }
            }
        }

        // Pisar clavos: cada uno le quita aire a la llanta y activa el aviso en pantalla.
        for (auto& cl : clavos) {
            if (!cl.activado) {
                float ds = std::abs(cl.s - sJugador);
                float dl = std::abs(cl.lateral - lateralJugador);
                if (ds < 0.9f && dl < 0.6f) {
                    cl.activado = true;
                    psi = std::max(0.0f, psi - psiPorClavo);
                    clavosAtropellados++;
                    alertaDanioTimer = 0.5f;
                    evClavo = true;
                }
            }
        }

        bool estabaInflando = inflandoAntes;

        // Recuerdos turísticos: se recogen pasando cerca del objeto dorado.
        for (size_t i = 0; i < recuerdos.size(); i++) {
            Recuerdo& r = recuerdos[i];
            if (r.recogido) continue;
            if (std::abs(r.s - sJugador) < 2.0f && std::abs(r.lateral - lateralJugador) < 1.6f) {
                r.recogido = true;
                recuerdosRecogidos++;
                evRecuerdo = (int)i;
            }
        }

        // Inflar llantas: si el ciclista casi se detiene junto a una bomba, la llanta se infla.
        for (auto& tal : talleres) {
            float ds = std::abs(tal.s - sJugador);
            float dl = std::abs(tal.lateral - lateralJugador);
            if (ds < 2.5f && dl < 2.2f && velocidadBici < 3.0f) {
                if (psi < PSI_MAX) {
                    psi = std::min(PSI_MAX, psi + 18.0f * dt);
                    if (!estabaInflando) evBombaInicio = true;
                    inflandoActualmente = true;
                }
            }
        }

        // Derrota: sin aire o sin agua se pierde una vida; sin vidas termina la partida.
        if (psi <= 0.0f) {
            psi = 0.0f;
            if (vidas > 1) {
                vidas--;
                psi = 22.0f;
                evVidaPerdida = true;
                avisoVidaTimer = 4.0f;
                avisoVidaTexto = "¡LLANTA PONCHADA! PERDISTE UNA VIDA. LLANTAS REPARADAS: 22 PSI";
            }
            else {
                vidas = 0;
                estado = STATUS_GAME_OVER_PONCHADURA;
                return 0.0f;
            }
        }
        if (agua <= 0.0f) {
            agua = 0.0f;
            if (vidas > 1) {
                vidas--;
                agua = 45.0f;
                evVidaPerdida = true;
                avisoVidaTimer = 4.0f;
                avisoVidaTexto = "¡DESHIDRATADO! PERDISTE UNA VIDA. TOMASTE AGUA: 45 %";
            }
            else {
                vidas = 0;
                estado = STATUS_GAME_OVER_DESHIDRATACION;
                return 0.0f;
            }
        }

        // Versión avanzada: se acaba el tiempo
        if (tiempoLimite > 0.0f && tiempoTotal >= tiempoLimite) {
            estado = STATUS_GAME_OVER_TIEMPO;
            return 0.0f;
        }

        // Victoria: se gana al llegar a la parte final de la ruta, dentro de la calle de Tepoztlán.
        bool enCalleTepoztlan = std::abs(lateralJugador) <= 2.2f;
        if (sJugador >= 480.0f && enCalleTepoztlan) {
            estado = STATUS_VICTORIA;
            return 0.0f;
        }

        // Qué tan bien rueda la bici según el aire de las llantas (1 es lo normal).
        if (psi < 12.0f) return 0.40f;
        if (psi < 22.0f) return 0.70f;
        return 1.0f;
    }

    // Dibuja en pantalla las botellas, los clavos y las bombas de aire.
    void DrawEntidades(Shader& shader, GLint modelLoc) {
        // Tiempo actual, se usa para animar las botellas.
        float t = (float)glfwGetTime();

        // Convierte una posición de la ruta (distancia y lado) en un punto del mundo 3D,
        // pegado al suelo.
        auto getCoord = [](float s, float lateral) {
            Ruta::Muestra p = Ruta::En(s);
            float th = glm::radians(p.headingDeg);
            glm::vec3 r = p.pos + glm::vec3(std::cos(th), 0.0f, -std::sin(th)) * lateral;
            r.y = Ruta::AlturaSuelo(r.x, r.z);
            return r;
            };

        GLint useColorLoc = glGetUniformLocation(shader.Program, "useColor");
        GLint meshColorLoc = glGetUniformLocation(shader.Program, "meshColor");
        // Estos objetos se pintan con un color sólido.
        glUniform1i(useColorLoc, 1);

        // 1. Botellas de agua: flotan y giran sobre el carril y se ven más grandes para que se noten.
        if (botellaModel) {
            glUniform3f(meshColorLoc, 0.15f, 0.75f, 1.0f); // Azul cian brillante
            for (const auto& anf : anforas) {
                if (!anf.recogida) {
                    glm::vec3 p = getCoord(anf.s, anf.lateral);
                    // La botella sube y baja suavemente.
                    float flotacion = std::sin(t * 3.5f + anf.s) * 0.06f;
                    p.y += 0.55f + flotacion; // Altura visible flotando en el carril

                    glm::mat4 m(1.0f);
                    m = glm::translate(m, p);
                    // Gira sobre sí misma.
                    m = glm::rotate(m, t * 2.5f, glm::vec3(0.0f, 1.0f, 0.0f));
                    m = glm::scale(m, glm::vec3(2.2f)); // Aumentada de 0.95f a 2.2f

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                    botellaModel->Draw(shader);
                }
            }
        }

        // 2. Clavos en el piso: se dibujan en rojo, grandes y alineados con la pista.
        if (clavosModel) {
            glUniform3f(meshColorLoc, 0.95f, 0.15f, 0.15f); // Rojo peligro
            for (const auto& cl : clavos) {
                if (!cl.activado) {
                    glm::vec3 p = getCoord(cl.s, cl.lateral);
                    Ruta::Muestra mRuta = Ruta::En(cl.s);

                    glm::mat4 m(1.0f);
                    // Se levantan apenas del asfalto para que no se mezclen con él visualmente
                    m = glm::translate(m, p + glm::vec3(0.0f, 0.09f, 0.0f));
                    m = glm::rotate(m, glm::radians(mRuta.headingDeg), glm::vec3(0.0f, 1.0f, 0.0f));
                    m = glm::scale(m, glm::vec3(4.5f)); // Incrementado a 4.5f

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                    clavosModel->Draw(shader);
                }
            }
        }

        // 3. Bombas de aire: a la orilla del camino, de frente a la pista.
        if (bombaModel) {
            glUniform3f(meshColorLoc, 1.0f, 0.85f, 0.10f); // Amarillo Truper
            for (const auto& tal : talleres) {
                glm::vec3 p = getCoord(tal.s, tal.lateral);
                Ruta::Muestra mRuta = Ruta::En(tal.s);

                glm::mat4 m(1.0f);
                m = glm::translate(m, p + glm::vec3(0.0f, 0.02f, 0.0f));
                // Se gira para quedar mirando hacia la pista.
                float giroLateral = (tal.lateral > 0.0f) ? -90.0f : 90.0f;
                m = glm::rotate(m, glm::radians(mRuta.headingDeg + giroLateral), glm::vec3(0.0f, 1.0f, 0.0f));
                m = glm::scale(m, glm::vec3(1.25f)); // Ligeramente más grande para facilitar verla

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                bombaModel->Draw(shader);
            }
        }

        // 4. Recuerdos turísticos: diamantes dorados que giran y flotan a la altura del ciclista
        if (recVAO) {
            glBindVertexArray(recVAO);
            for (const auto& r : recuerdos) {
                if (r.recogido) continue;
                glm::vec3 p = getCoord(r.s, r.lateral);
                p.y += 1.15f + std::sin(t * 2.5f + r.s) * 0.12f;
                // Pulso suave de brillo
                float br = 0.85f + 0.15f * std::sin(t * 4.0f + r.s);
                glUniform3f(meshColorLoc, 1.0f * br, 0.78f * br, 0.10f * br);

                glm::mat4 m(1.0f);
                m = glm::translate(m, p);
                m = glm::rotate(m, t * 1.8f, glm::vec3(0.0f, 1.0f, 0.0f));
                m = glm::scale(m, glm::vec3(0.75f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                glDrawArrays(GL_TRIANGLES, 0, recVerts);

                // Pequeño diamante más grande y claro debajo, para que se note desde lejos
                glUniform3f(meshColorLoc, 1.0f, 0.95f, 0.55f);
                m = glm::mat4(1.0f);
                m = glm::translate(m, p + glm::vec3(0.0f, 1.1f, 0.0f));
                m = glm::rotate(m, -t * 1.2f, glm::vec3(0.0f, 1.0f, 0.0f));
                m = glm::scale(m, glm::vec3(0.28f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                glDrawArrays(GL_TRIANGLES, 0, recVerts);
            }
            glBindVertexArray(0);
        }

        glUniform1i(useColorLoc, 0);
    }
};