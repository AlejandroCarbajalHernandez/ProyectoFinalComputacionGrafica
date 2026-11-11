#pragma once
// AvesPose.h - Pose de las aves: cuerpo + dos alas (jerarquia) con aleteo por KEYFRAMES.
// El modelo bird.obj es una sola malla; cada vertice se asigna a un hueso segun su posicion:
// CUERPO (se mueve con la trayectoria) y las dos alas (giran sobre una bisagra en el hombro).
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace AvesPose {

enum Bone { CUERPO = 0, ALA_IZQ, ALA_DER, BONE_COUNT };

// Medidas del modelo original (bird.obj, sin escalar). El ave mira hacia -z y las alas se abren en x.
const float CX = 0.93f;            // centro del cuerpo en x
const float BISAGRA_DX = 2.0f;     // las alas empiezan a 2 unidades del centro
const float BISAGRA_Y = 3.4f;      // altura del hombro
const float BISAGRA_Z = 0.7f;      // posicion del hombro a lo largo del cuerpo

// Hueso al que pertenece un vertice
inline float BoneForVertex(const glm::vec3& p) {
    float dx = p.x - CX;
    if (dx < -BISAGRA_DX) return (float)ALA_IZQ;
    if (dx >  BISAGRA_DX) return (float)ALA_DER;
    return (float)CUERPO;
}

// ---- Keyframes del aleteo: angulo del ala (grados) en cada instante del ciclo ----
struct Keyframe { float t; float grados; };

inline const std::vector<Keyframe>& Aleteo() {
    static const std::vector<Keyframe> k = {
        { 0.00f,  25.0f },   // alas arriba
        { 0.20f,   0.0f },   // bajando
        { 0.35f, -40.0f },   // alas abajo
        { 0.55f, -10.0f },   // subiendo
        { 0.75f,  25.0f }    // otra vez arriba (cierra el ciclo)
    };
    return k;
}
const float PERIODO = 0.75f;   // duracion de un aleteo (s)

// Interpola entre keyframes con una curva Catmull-Rom que se repite (ciclo cerrado)
inline float Muestrear(float tiempo) {
    const std::vector<Keyframe>& k = Aleteo();
    int n = (int)k.size() - 1;                 // el ultimo keyframe repite al primero
    float t = std::fmod(tiempo, PERIODO);
    if (t < 0.0f) t += PERIODO;
    int i = 0;
    while (i < n - 1 && t > k[i + 1].t) i++;
    float u = (t - k[i].t) / (k[i + 1].t - k[i].t);
    float p0 = k[(i - 1 + n) % n].grados, p1 = k[i].grados;
    float p2 = k[(i + 1) % n].grados,     p3 = k[(i + 2) % n].grados;
    return 0.5f * ((2.0f * p1) + (-p0 + p2) * u + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * u * u
                   + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * u * u * u);
}

inline glm::mat4 AlrededorDe(const glm::mat4& R, const glm::vec3& pivote) {
    return glm::translate(glm::mat4(1.0f), pivote) * R * glm::translate(glm::mat4(1.0f), -pivote);
}

// Matrices de los 3 huesos para un instante del aleteo
inline void Compute(float tiempo, glm::mat4 out[BONE_COUNT]) {
    float a = glm::radians(Muestrear(tiempo));
    glm::vec3 z(0.0f, 0.0f, 1.0f);
    out[CUERPO] = glm::mat4(1.0f);
    out[ALA_DER] = AlrededorDe(glm::rotate(glm::mat4(1.0f),  a, z), glm::vec3(CX + BISAGRA_DX, BISAGRA_Y, BISAGRA_Z));
    out[ALA_IZQ] = AlrededorDe(glm::rotate(glm::mat4(1.0f), -a, z), glm::vec3(CX - BISAGRA_DX, BISAGRA_Y, BISAGRA_Z));
}

} // namespace AvesPose
