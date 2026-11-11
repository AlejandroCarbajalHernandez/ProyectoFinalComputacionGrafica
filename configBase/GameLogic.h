#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include "Ruta.h"
#include "Shader.h"

enum GameStatus {
    STATUS_JUGANDO,
    STATUS_GAME_OVER_PONCHADURA,
    STATUS_GAME_OVER_DESHIDRATACION,
    STATUS_VICTORIA
};

struct AnforaAgua {
    float s;
    float lateral;
    bool recogida;
};

struct PeligroClavos {
    float s;
    float lateral;
    bool activado;
};

struct TallerInflado {
    float s;
    float lateral;
    bool activo;
};

class GameLogic {
public:
    float psi;
    const float PSI_MAX = 35.0f;
    float agua;
    const float AGUA_MAX = 100.0f;
    int anforasRecogidas;
    int clavosAtropellados;
    float tiempoTotal;
    GameStatus estado;

    std::vector<AnforaAgua> anforas;
    std::vector<PeligroClavos> clavos;
    std::vector<TallerInflado> talleres;

    float alertaDanioTimer;
    bool inflandoActualmente;

    GLuint itemVAO = 0;
    GLuint itemVBO = 0;

    GameLogic() {
        Reiniciar();
    }

    ~GameLogic() {
        if (itemVAO) glDeleteVertexArrays(1, &itemVAO);
        if (itemVBO) glDeleteBuffers(1, &itemVBO);
    }

    void Init() {
        float vertices[] = {
            // Cara Frontal
            -0.5f, -0.5f,  0.5f,      0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,      0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,      0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,      0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,      0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,      0.0f,  0.0f,  1.0f,   0.0f, 0.0f,

            // Cara Trasera
            -0.5f, -0.5f, -0.5f,      0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,      0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,      0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,      0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,      0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,      0.0f,  0.0f, -1.0f,   0.0f, 1.0f,

            // Cara Izquierda
            -0.5f,  0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,     -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,     -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

            // Cara Derecha
             0.5f,  0.5f,  0.5f,      1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,      1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,      1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,      1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,      1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,      1.0f,  0.0f,  0.0f,   0.0f, 0.0f,

             // Cara Inferior
            -0.5f, -0.5f, -0.5f,      0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,      0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,      0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,      0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,      0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,      0.0f, -1.0f,  0.0f,   0.0f, 1.0f,

            // Cara Superior
           -0.5f,  0.5f, -0.5f,      0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
            0.5f,  0.5f,  0.5f,      0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,      0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,      0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
           -0.5f,  0.5f, -0.5f,      0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
           -0.5f,  0.5f,  0.5f,      0.0f,  1.0f,  0.0f,   0.0f, 0.0f
        };

        glGenVertexArrays(1, &itemVAO);
        glGenBuffers(1, &itemVBO);
        glBindVertexArray(itemVAO);
        glBindBuffer(GL_ARRAY_BUFFER, itemVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

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
        anforas.push_back({ 25.0f,  0.3f, false });
        anforas.push_back({ 75.0f, -0.4f, false });
        anforas.push_back({ 150.0f,  0.4f, false });
        anforas.push_back({ 240.0f, -0.3f, false });
        anforas.push_back({ 310.0f,  0.3f, false });
        anforas.push_back({ 365.0f, -0.4f, false });
        anforas.push_back({ 425.0f,  0.2f, false });

        // 2. Clavos / Peligros
        clavos.push_back({ 55.0f,  0.2f, false });
        clavos.push_back({ 120.0f, -0.3f, false });
        clavos.push_back({ 270.0f,  0.4f, false });
        clavos.push_back({ 340.0f, -0.2f, false });
        clavos.push_back({ 400.0f,  0.3f, false });

        // 3. Estaciones de inflado
        talleres.push_back({ 90.0f,  1.7f, true });
        talleres.push_back({ 200.0f, -1.7f, true });
        talleres.push_back({ 350.0f,  1.8f, true });
        talleres.push_back({ 415.0f, -1.8f, true });
    }

    float Actualizar(float dt, float sJugador, float lateralJugador, float velocidadBici = 0.0f, float pendienteRuta = 0.0f) {
        if (estado != STATUS_JUGANDO) return 0.0f;

        tiempoTotal += dt;
        if (alertaDanioTimer > 0.0f) alertaDanioTimer -= dt;
        inflandoActualmente = false;

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

        agua -= (1.2f + (velocidadBici * 0.06f)) * dt;

        // Recolección de agua
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

        // Colisión con clavos
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

        // Inflado en taller
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

        // Candado original para Tepoztlán
        bool enCalleTepoztlan = std::abs(lateralJugador) <= 2.2f;
        if (sJugador >= 480.0f && enCalleTepoztlan) {
            estado = STATUS_VICTORIA;
            return 0.0f;
        }

        if (psi < 12.0f) return 0.40f;
        if (psi < 22.0f) return 0.70f;
        return 1.0f;
    }

    void DrawEntidades(Shader& shader, GLint modelLoc) {
        if (!itemVAO) return;

        float t = (float)glfwGetTime();

        auto getCoord = [](float s, float lateral) {
            Ruta::Muestra p = Ruta::En(s);
            float th = glm::radians(p.headingDeg);
            glm::vec3 r = p.pos + glm::vec3(std::cos(th), 0.0f, -std::sin(th)) * lateral;
            r.y = Ruta::AlturaSuelo(r.x, r.z);
            return r;
            };

        glUniform1i(glGetUniformLocation(shader.Program, "useColor"), 1);
        glBindVertexArray(itemVAO);

        // 1. Ánforas de agua
        for (const auto& anf : anforas) {
            if (!anf.recogida) {
                glm::vec3 p = getCoord(anf.s, anf.lateral);
                float flotacion = std::sin(t * 4.0f + anf.s) * 0.08f;
                p.y += 0.50f + flotacion;

                glm::mat4 m = glm::mat4(1.0f);
                m = glm::translate(m, p);
                m = glm::rotate(m, t * 2.5f, glm::vec3(0.0f, 1.0f, 0.0f));
                m = glm::scale(m, glm::vec3(0.24f, 0.45f, 0.24f));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), 0.10f, 0.65f, 0.95f);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        // 2. Clavos / Trampas
        for (const auto& cl : clavos) {
            if (!cl.activado) {
                glm::vec3 p = getCoord(cl.s, cl.lateral);
                p.y += 0.04f;

                glm::mat4 m = glm::mat4(1.0f);
                m = glm::translate(m, p);
                m = glm::scale(m, glm::vec3(0.35f, 0.05f, 0.35f));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), 0.90f, 0.10f, 0.10f);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        // 3. Estaciones de inflado
        for (const auto& tal : talleres) {
            glm::vec3 p = getCoord(tal.s, tal.lateral);
            p.y += 0.70f;

            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, p);
            m = glm::scale(m, glm::vec3(0.35f, 1.40f, 0.35f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), 0.95f, 0.85f, 0.10f);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glBindVertexArray(0);
        glUniform1i(glGetUniformLocation(shader.Program, "useColor"), 0);
    }
};