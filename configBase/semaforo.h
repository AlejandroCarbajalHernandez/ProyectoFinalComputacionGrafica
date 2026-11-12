#pragma once
// ============================================================================
// Semaforo.h - Clase unificada de Semaforo:
// Soporta tanto la instancia orientada a objetos (PolancoScene) como
// las constantes y funciones de tiempo globales (main.cpp y CicloviaScene).
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

    // ------------------------------------------------------------------------
    // Compatibilidad estática con el juego global (main.cpp y CicloviaScene)
    // ------------------------------------------------------------------------
    static constexpr float T_VERDE = 6.0f;
    static constexpr float T_AMBAR = 2.0f;
    static constexpr float T_ROJO = 6.0f;
    static constexpr float POSICIONES[2] = { 55.0f, 190.0f };

    // Función estática global: evalúa el ciclo general de la ciclovía
    static inline Fase FaseEn(float t) {
        float c = std::fmod(t, T_VERDE + T_AMBAR + T_ROJO);
        if (c < 0.0f) c += (T_VERDE + T_AMBAR + T_ROJO);
        if (c < T_VERDE) return VERDE;
        if (c < T_VERDE + T_AMBAR) return AMBAR;
        return ROJO;
    }

    // ------------------------------------------------------------------------
    // Miembros de instancia para PolancoScene (Cruce y animación procedural)
    // ------------------------------------------------------------------------
    float tVerde = 8.0f;
    float tAmbar = 2.0f;
    float tRojo = 8.0f;
    float parpadeoFinal = 1.5f;

    glm::vec3 luzRoja = glm::vec3(1.00f, 0.12f, 0.10f);
    glm::vec3 luzAmbar = glm::vec3(1.00f, 0.70f, 0.05f);
    glm::vec3 luzVerde = glm::vec3(0.10f, 1.00f, 0.35f);

    Model* modelo = nullptr;

    Semaforo() {}
    ~Semaforo() { delete modelo; }
    Semaforo(const Semaforo&) = delete;
    Semaforo& operator=(const Semaforo&) = delete;

    bool Init(const char* ruta = "../assets/obj/Polanco/Semaforo.obj") {
        modelo = new Model(const_cast<GLchar*>(ruta));
        return modelo != nullptr;
    }

    float Periodo() const { return tVerde + tAmbar + tRojo; }

    // Función de instancia: evalúa el ciclo específico de este semáforo con desfase
    Fase FaseEn(float tiempo, float desfase, float* restante = nullptr) const {
        float p = Periodo();
        float t = std::fmod(tiempo + desfase, p);
        if (t < 0.0f) t += p;
        if (t < tVerde) {
            if (restante) *restante = tVerde - t;
            return VERDE;
        }
        if (t < tVerde + tAmbar) {
            if (restante) *restante = tVerde + tAmbar - t;
            return AMBAR;
        }
        if (restante) *restante = p - t;
        return ROJO;
    }

    void Draw(Shader& shader, GLint modelLoc, const glm::mat4& model,
        float tiempo, float desfase = 0.0f) const {
        if (!modelo) return;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        GLint locE = glGetUniformLocation(shader.Program, "emissive");

        float resto = 0.0f;
        Fase fase = FaseEn(tiempo, desfase, &resto);

        // Caja, poste y viseras: sin emisión
        glUniform3f(locE, 0.0f, 0.0f, 0.0f);
        modelo->DrawMatching(shader, "SemCuerpo");

        // Parpadeo de la luz verde en la transición
        bool verdeVisible = true;
        if (fase == VERDE && parpadeoFinal > 0.0f && resto < parpadeoFinal) {
            verdeVisible = (static_cast<int>(resto * 4.0f) % 2) == 0;
        }

        EncenderYDibujar(shader, locE, "SemLuzRoja", luzRoja, fase == ROJO);
        EncenderYDibujar(shader, locE, "SemLuzAmbar", luzAmbar, fase == AMBAR);
        EncenderYDibujar(shader, locE, "SemLuzVerde", luzVerde, fase == VERDE && verdeVisible);

        glUniform3f(locE, 0.0f, 0.0f, 0.0f);
    }

private:
    void EncenderYDibujar(Shader& shader, GLint locE, const char* clave,
        const glm::vec3& color, bool encendido) const {
        if (encendido) glUniform3f(locE, color.r, color.g, color.b);
        else           glUniform3f(locE, 0.0f, 0.0f, 0.0f);
        modelo->DrawMatching(shader, clave);
    }
};