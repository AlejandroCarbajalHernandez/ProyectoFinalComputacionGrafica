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
    STATUS_VICTORIA
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
    // Botellas recogidas (suman puntos)
    int anforasRecogidas;
    // Clavos pisados (restan puntos)
    int clavosAtropellados;
    // Segundos transcurridos en la partida
    float tiempoTotal;
    // Si se sigue jugando, se ganó o se perdió
    GameStatus estado;

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

    // Al crear el juego se dejan los modelos vacíos y se arma una partida nueva.
    GameLogic() {
        botellaModel = nullptr;
        clavosModel = nullptr;
        bombaModel = nullptr;
        Reiniciar();
    }

    // Libera los modelos de memoria al cerrar el programa.
    ~GameLogic() {
        delete botellaModel;
        delete clavosModel;
        delete bombaModel;
    }

    // Carga de modelos (debe ejecutarse en main DESPUÉS de glewInit)
    void Init() {
        botellaModel = new Model((char*)"../assets/obj/botella/botella.obj");
        clavosModel = new Model((char*)"../assets/obj/Clavos/clavos.obj");
        bombaModel = new Model((char*)"../assets/obj/bomba-aire/bomba-aire.obj");

        std::cout << "\n========================================" << std::endl;
        std::cout << "[GameLogic] Carga de modelos completada." << std::endl;
        std::cout << "========================================\n" << std::endl;
    }

    // Deja todo como al inicio: llantas y agua llenas, contadores en cero
    // y todos los objetos del camino colocados de nuevo en su lugar.
    void Reiniciar() {
        psi = PSI_MAX;
        agua = AGUA_MAX;
        anforasRecogidas = 0;
        clavosAtropellados = 0;
        tiempoTotal = 0.0f;
        alertaDanioTimer = 0.0f;
        inflandoActualmente = false;
        estado = STATUS_JUGANDO;

        anforas.clear();
        clavos.clear();
        talleres.clear();

        // 1. Ánforas de agua
        anforas.push_back({ 25.0f,   0.3f, false });
        anforas.push_back({ 75.0f,  -0.4f, false });
        anforas.push_back({ 150.0f,  0.4f, false });
        anforas.push_back({ 240.0f, -0.3f, false });
        anforas.push_back({ 310.0f,  0.3f, false });
        anforas.push_back({ 365.0f, -0.4f, false });
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

        tiempoTotal += dt;
        if (alertaDanioTimer > 0.0f) alertaDanioTimer -= dt;
        inflandoActualmente = false;

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

        // Consumo de agua: se gasta poco a poco y más rápido mientras más veloz va el ciclista.
        agua -= (1.2f + (velocidadBici * 0.06f)) * dt;

        // Recoger botellas: si el ciclista pasa muy cerca de una, la recoge y recupera agua.
        for (auto& anf : anforas) {
            if (!anf.recogida) {
                float ds = std::abs(anf.s - sJugador);
                float dl = std::abs(anf.lateral - lateralJugador);
                if (ds < 1.3f && dl < 0.8f) {
                    anf.recogida = true;
                    agua = std::min(AGUA_MAX, agua + 25.0f);
                    anforasRecogidas++;
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
                    psi = std::max(0.0f, psi - 12.0f);
                    clavosAtropellados++;
                    alertaDanioTimer = 0.5f;
                }
            }
        }

        // Inflar llantas: si el ciclista casi se detiene junto a una bomba, la llanta se infla.
        for (auto& tal : talleres) {
            float ds = std::abs(tal.s - sJugador);
            float dl = std::abs(tal.lateral - lateralJugador);
            if (ds < 2.5f && dl < 2.2f && velocidadBici < 3.0f) {
                if (psi < PSI_MAX) {
                    psi = std::min(PSI_MAX, psi + 18.0f * dt);
                    inflandoActualmente = true;
                }
            }
        }

        // Derrota: sin aire en las llantas o sin agua termina la partida.
        if (psi <= 0.0f) {
            psi = 0.0f;
            estado = STATUS_GAME_OVER_PONCHADURA;
            return 0.0f;
        }
        if (agua <= 0.0f) {
            agua = 0.0f;
            estado = STATUS_GAME_OVER_DESHIDRATACION;
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

        glUniform1i(useColorLoc, 0);
    }
};