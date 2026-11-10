#pragma once
// ============================================================================

#include <cmath>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Model.h"

class Semaforo {
public:
    enum Fase { VERDE = 0, AMBAR = 1, ROJO = 2 };

    // Duraciones del ciclo, en segundos
    float tVerde = 8.0f;
    float tAmbar = 2.0f;
    float tRojo = 8.0f;
    float parpadeoFinal = 1.5f;     // el verde parpadea durante los últimos s (0 = sin parpadeo)

    // Color con el que brilla cada foco al encenderse
    glm::vec3 luzRoja = glm::vec3(1.00f, 0.12f, 0.10f);
    glm::vec3 luzAmbar = glm::vec3(1.00f, 0.70f, 0.05f);
    glm::vec3 luzVerde = glm::vec3(0.10f, 1.00f, 0.35f);

    Model* modelo = nullptr;

    Semaforo() {}
    ~Semaforo() { delete modelo; }
    Semaforo(const Semaforo&) = delete;
    Semaforo& operator=(const Semaforo&) = delete;

    // Carga el modelo. Devuelve false si no se pudo.
    bool Init(const char* ruta = "../assets/obj/Polanco/Semaforo.obj") {
        modelo = new Model(const_cast<GLchar*>(ruta));
        return modelo != nullptr;
    }

    float Periodo() const { return tVerde + tAmbar + tRojo; }

    // Fase en el instante `tiempo` (s). `desfase` adelanta o atrasa este semáforo respecto a otro.
    // `restante` (opcional) devuelve los segundos que faltan para que cambie de fase.
    Fase FaseEn(float tiempo, float desfase = 0.0f, float* restante = nullptr) const {
        float p = Periodo();
        float t = std::fmod(tiempo + desfase, p);
        if (t < 0.0f) t += p;
        if (t < tVerde) { if (restante) *restante = tVerde - t; return VERDE; }
        if (t < tVerde + tAmbar) { if (restante) *restante = tVerde + tAmbar - t; return AMBAR; }
        if (restante) *restante = p - t;
        return ROJO;
    }

    // Dibuja el semáforo con la matriz `model` (posición, giro y escala) en el instante `tiempo`.
    // Para dos semáforos que se alternan (cruce), usa desfase = tVerde + tAmbar en uno de ellos.
    void Draw(Shader& shader, GLint modelLoc, const glm::mat4& model,
        float tiempo, float desfase = 0.0f) const {
        if (!modelo) return;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        GLint locE = glGetUniformLocation(shader.Program, "emissive");

        float resto = 0.0f;
        Fase fase = FaseEn(tiempo, desfase, &resto);

        // Caja, poste y viseras: sin brillo propio
        glUniform3f(locE, 0.0f, 0.0f, 0.0f);
        modelo->DrawMatching(shader, "SemCuerpo");

        // Parpadeo del verde al final de su fase (4 cambios por segundo)
        bool verdeVisible = true;
        if (fase == VERDE && parpadeoFinal > 0.0f && resto < parpadeoFinal)
            verdeVisible = (static_cast<int>(resto * 4.0f) % 2) == 0;

        EncenderYDibujar(shader, locE, "SemLuzRoja", luzRoja, fase == ROJO);
        EncenderYDibujar(shader, locE, "SemLuzAmbar", luzAmbar, fase == AMBAR);
        EncenderYDibujar(shader, locE, "SemLuzVerde", luzVerde, fase == VERDE && verdeVisible);

        glUniform3f(locE, 0.0f, 0.0f, 0.0f);   // deja el shader como estaba para el resto de la escena
    }

private:
    void EncenderYDibujar(Shader& shader, GLint locE, const char* clave,
        const glm::vec3& color, bool encendido) const {
        if (encendido) glUniform3f(locE, color.r, color.g, color.b);
        else           glUniform3f(locE, 0.0f, 0.0f, 0.0f);
        modelo->DrawMatching(shader, clave);
    }
};

