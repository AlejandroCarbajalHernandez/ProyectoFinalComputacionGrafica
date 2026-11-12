#pragma once
// Semaforo.h - Clase unificada de Semaforo:
// Soporta tanto la instancia orientada a objetos (PolancoScene) como
// las constantes y funciones de tiempo globales (main.cpp y CicloviaScene).

#include <cmath>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Model.h"

// Semáforo: cambia de verde a ámbar y a rojo, y enciende en su modelo 3D la luz que corresponde.
class Semaforo {
public:
    // Las tres luces posibles del semáforo.
    enum Fase { VERDE = 0, AMBAR = 1, ROJO = 2 };

    // Parte que usa el juego completo (main.cpp y CicloviaScene)
    // Segundos que dura cada luz en los semáforos de la ciclovía.
    static constexpr float T_VERDE = 6.0f;
    static constexpr float T_AMBAR = 2.0f;
    static constexpr float T_ROJO = 6.0f;
    // Distancia (en metros desde la salida) donde están los dos semáforos de la ruta.
    static constexpr float POSICIONES[2] = { 55.0f, 190.0f };

    // Función estática global: evalúa el ciclo general de la ciclovía
    // Dado el tiempo, dice qué luz está encendida. El ciclo se repite una y otra vez.
    static inline Fase FaseEn(float t) {
        // Se calcula en qué punto del ciclo estamos.
        float c = std::fmod(t, T_VERDE + T_AMBAR + T_ROJO);
        if (c < 0.0f) c += (T_VERDE + T_AMBAR + T_ROJO);
        // Según el punto del ciclo se regresa verde, ámbar o rojo.
        if (c < T_VERDE) return VERDE;
        if (c < T_VERDE + T_AMBAR) return AMBAR;
        return ROJO;
    }

    // Parte que usa la escena de Polanco (el cruce con semáforo)
    // Segundos que dura cada luz en el semáforo del cruce de Polanco.
    float tVerde = 8.0f;
    float tAmbar = 2.0f;
    float tRojo = 8.0f;
    // Segundos finales de la luz verde en los que parpadea para avisar que va a cambiar.
    float parpadeoFinal = 1.5f;

    // Color de cada luz cuando está encendida.
    glm::vec3 luzRoja = glm::vec3(1.00f, 0.12f, 0.10f);
    glm::vec3 luzAmbar = glm::vec3(1.00f, 0.70f, 0.05f);
    glm::vec3 luzVerde = glm::vec3(0.10f, 1.00f, 0.35f);

    // Modelo 3D del semáforo.
    Model* modelo = nullptr;

    // Al destruir el semáforo se libera su modelo de la memoria.
    Semaforo() {}
    ~Semaforo() { delete modelo; }
    // No se permite copiar un semáforo, para no duplicar su modelo en memoria.
    Semaforo(const Semaforo&) = delete;
    Semaforo& operator=(const Semaforo&) = delete;

    // Carga el modelo 3D del semáforo desde su archivo.
    bool Init(const char* ruta = "../assets/obj/Polanco/Semaforo.obj") {
        modelo = new Model(const_cast<GLchar*>(ruta));
        return modelo != nullptr;
    }

    // Duración total de un ciclo completo (verde, ámbar y rojo).
    float Periodo() const { return tVerde + tAmbar + tRojo; }

    // Función de instancia: evalúa el ciclo específico de este semáforo con desfase
    // El desfase permite que cada semáforo esté en un momento distinto del ciclo.
    // Opcionalmente dice cuántos segundos le quedan a la luz actual.
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

    // Dibuja el semáforo en su lugar y enciende solo la luz que toca según el momento.
    void Draw(Shader& shader, GLint modelLoc, const glm::mat4& model,
        float tiempo, float desfase = 0.0f) const {
        if (!modelo) return;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        // Dato que usa el dibujo para hacer brillar las luces.
        GLint locE = glGetUniformLocation(shader.Program, "emissive");

        // Se averigua qué luz toca y cuánto tiempo le queda.
        float resto = 0.0f;
        Fase fase = FaseEn(tiempo, desfase, &resto);

        // Caja, poste y viseras: no brillan
        glUniform3f(locE, 0.0f, 0.0f, 0.0f);
        modelo->DrawMatching(shader, "SemCuerpo");

        // Parpadeo de la luz verde en la transición
        bool verdeVisible = true;
        if (fase == VERDE && parpadeoFinal > 0.0f && resto < parpadeoFinal) {
            verdeVisible = (static_cast<int>(resto * 4.0f) % 2) == 0;
        }

        // Cada luz se prende solo cuando es su turno.
        EncenderYDibujar(shader, locE, "SemLuzRoja", luzRoja, fase == ROJO);
        EncenderYDibujar(shader, locE, "SemLuzAmbar", luzAmbar, fase == AMBAR);
        EncenderYDibujar(shader, locE, "SemLuzVerde", luzVerde, fase == VERDE && verdeVisible);

        // Se apaga el brillo para que no afecte a los demás objetos.
        glUniform3f(locE, 0.0f, 0.0f, 0.0f);
    }

private:
    // Prende o apaga una luz y la dibuja.
    void EncenderYDibujar(Shader& shader, GLint locE, const char* clave,
        const glm::vec3& color, bool encendido) const {
        // Encendida brilla con su color; apagada se queda sin brillo.
        if (encendido) glUniform3f(locE, color.r, color.g, color.b);
        else           glUniform3f(locE, 0.0f, 0.0f, 0.0f);
        modelo->DrawMatching(shader, clave);
    }
};