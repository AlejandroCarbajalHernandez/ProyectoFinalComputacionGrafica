#pragma once
// Semaforo.h - Ciclo de los semaforos de la zona urbana, compartido entre el dibujo (CicloviaScene)
// y las reglas del juego (main.cpp), para que siempre coincidan.
#include <cmath>

namespace Semaforo {

enum Fase { VERDE, AMBAR, ROJO };

const float T_VERDE = 6.0f;                 // segundos en verde
const float T_AMBAR = 2.0f;                 // segundos en ambar
const float T_ROJO  = 6.0f;                 // segundos en rojo
const float POSICIONES[2] = { 55.0f, 190.0f };   // metro de la ruta donde esta cada semaforo

// Fase del ciclo en el instante t (segundos)
inline Fase FaseEn(float t) {
    float c = std::fmod(t, T_VERDE + T_AMBAR + T_ROJO);
    if (c < T_VERDE) return VERDE;
    if (c < T_VERDE + T_AMBAR) return AMBAR;
    return ROJO;
}

} // namespace Semaforo
