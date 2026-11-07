#pragma once
// Pose del ciclista por segmentos rigidos (sin huesos en el .obj).
// Todo se calcula en el espacio local del modelo del ciclista (1 unidad = 1 unidad del .obj).
// Este archivo solo usa GLM: no depende de OpenGL.
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <algorithm>

namespace RiderPose {

enum Bone { BODY = 0, THIGH_A, SHIN_A, THIGH_B, SHIN_B, UPPER_A, FORE_A, UPPER_B, FORE_B, HEAD, BONE_COUNT };
// Lado A = x local positivo, lado B = x local negativo.

// ---- Medidas del cuerpo (tomadas del .obj del ciclista) ----
const float HIP_Y = 0.98f, KNEE_Y = 0.52f, ANKLE_Y = 0.10f, LEG_Z = 0.03f;
const float LEG_X = 0.14f;
const glm::vec3 SHOULDER_REST(0.30f, 1.56f, 0.03f);   // lado A
const glm::vec3 HAND_REST(0.64f, 1.16f, 0.35f);       // centro de la mano, lado A
const glm::vec3 NECK(0.0f, 1.70f, 0.03f);             // base de la cabeza
const float HEAD_COMPENSATION = 0.75f;                // cuanto se levanta la cabeza respecto a la inclinacion del torso

// ---- Posicion del ciclista sobre la bicicleta (espacio de la bicicleta) ----
const glm::vec3 RIDER_OFFSET(0.0f, -0.30f, 0.21f);    // traslacion del modelo del ciclista
const float TORSO_LEAN_DEG = 46.0f;                   // inclinacion del torso hacia el manubrio
const float CRANK_R = 0.17f;                          // largo de la biela
const float PEDAL_Z = 0.21f;                          // posicion del pedalier (en z local)
const float PEDAL_ANKLE_Y = 0.31f;                    // altura del tobillo cuando la biela esta horizontal
const glm::vec3 GRIP_LOCAL(0.17f, 1.03f, 0.80f);      // mano en el manubrio, lado A (x se invierte en lado B)

inline float BoneForVertex(const glm::vec3& p) {
    bool A = p.x >= 0.0f;
    if (p.y < 0.95f) {                                // piernas
        bool muslo = p.y >= KNEE_Y;
        if (A) return muslo ? THIGH_A : SHIN_A;
        return muslo ? THIGH_B : SHIN_B;
    }
    if (std::fabs(p.x) > 0.28f && p.y < 1.66f) {     // brazos
        float sx = A ? 1.0f : -1.0f;
        glm::vec3 S(sx * SHOULDER_REST.x, SHOULDER_REST.y, SHOULDER_REST.z);
        glm::vec3 H(sx * HAND_REST.x, HAND_REST.y, HAND_REST.z);
        glm::vec3 a = H - S;
        float t = glm::dot(p - S, a) / glm::dot(a, a);
        bool alto = t < 0.5f;
        if (A) return alto ? UPPER_A : FORE_A;
        return alto ? UPPER_B : FORE_B;
    }
    if (p.y >= 1.72f && std::fabs(p.x) < 0.22f) return HEAD;   // cabeza y cabello
    return BODY;
}

inline glm::mat4 RotBetween(glm::vec3 a, glm::vec3 b) {
    a = glm::normalize(a); b = glm::normalize(b);
    glm::vec3 c = glm::cross(a, b);
    float s = glm::length(c), d = glm::dot(a, b);
    if (s < 1e-6f) return glm::mat4(1.0f);
    return glm::rotate(glm::mat4(1.0f), std::atan2(s, d), c / s);
}

inline glm::mat4 AboutPoint(const glm::mat4& R, const glm::vec3& p) {
    return glm::translate(glm::mat4(1.0f), p) * R * glm::translate(glm::mat4(1.0f), -p);
}

// Pierna: IK plana en (z, y). Devuelve los angulos absolutos del muslo y la pantorrilla.
inline void LegIK(const glm::vec2& hip, const glm::vec2& target, float L1, float L2, float& phi1, float& phi2) {
    glm::vec2 v = target - hip;
    float d = glm::length(v);
    float dmax = L1 + L2 - 0.002f, dmin = std::fabs(L1 - L2) + 0.002f;
    if (d > dmax) { v *= dmax / d; d = dmax; }
    if (d < dmin) { v *= dmin / d; d = dmin; }
    glm::vec2 u = v / d;
    float alpha = std::acos(glm::clamp((L1 * L1 + d * d - L2 * L2) / (2.0f * L1 * d), -1.0f, 1.0f));
    glm::vec2 perp(-u.y, u.x);
    glm::vec2 k1 = hip + L1 * (std::cos(alpha) * u + std::sin(alpha) * perp);
    glm::vec2 k2 = hip + L1 * (std::cos(alpha) * u - std::sin(alpha) * perp);
    glm::vec2 knee = (k1.x > k2.x) ? k1 : k2;         // la rodilla se dobla hacia adelante (+z)
    glm::vec2 d1 = knee - hip;
    glm::vec2 d2 = (hip + v) - knee;
    phi1 = std::atan2(-d1.x, -d1.y);                  // x = z, y = y
    phi2 = std::atan2(-d2.x, -d2.y);
}

// pedalAngle: angulo de la biela del lado A en radianes (0 = biela hacia adelante).
inline void Compute(float pedalAngle, glm::mat4 out[BONE_COUNT]) {
    for (int i = 0; i < BONE_COUNT; i++) out[i] = glm::mat4(1.0f);

    // Torso: gira hacia adelante sobre la cadera
    glm::vec3 hipC(0.0f, HIP_Y, LEG_Z);
    glm::mat4 B0 = AboutPoint(glm::rotate(glm::mat4(1.0f), glm::radians(TORSO_LEAN_DEG), glm::vec3(1, 0, 0)), hipC);
    out[BODY] = B0;

    // Cabeza: se levanta para mirar al frente aunque el torso vaya inclinado
    out[HEAD] = B0 * AboutPoint(glm::rotate(glm::mat4(1.0f), -glm::radians(TORSO_LEAN_DEG) * HEAD_COMPENSATION, glm::vec3(1, 0, 0)), NECK);

    // Piernas
    for (int side = 0; side < 2; side++) {
        float sx = side == 0 ? 1.0f : -1.0f;
        float ang = pedalAngle + (side == 0 ? 0.0f : 3.14159265f);
        glm::vec2 hip(LEG_Z, HIP_Y);
        glm::vec2 target(PEDAL_Z + CRANK_R * std::cos(ang), PEDAL_ANKLE_Y + CRANK_R * std::sin(ang));
        float L1 = HIP_Y - KNEE_Y, L2 = KNEE_Y - ANKLE_Y, phi1, phi2;
        LegIK(hip, target, L1, L2, phi1, phi2);
        glm::vec3 hipJ(sx * LEG_X, HIP_Y, LEG_Z), kneeJ(sx * LEG_X, KNEE_Y, LEG_Z);
        glm::mat4 Bt = AboutPoint(glm::rotate(glm::mat4(1.0f), phi1, glm::vec3(1, 0, 0)), hipJ);
        glm::mat4 Bs = Bt * AboutPoint(glm::rotate(glm::mat4(1.0f), phi2 - phi1, glm::vec3(1, 0, 0)), kneeJ);
        out[side == 0 ? THIGH_A : THIGH_B] = Bt;
        out[side == 0 ? SHIN_A : SHIN_B] = Bs;
    }

    // Brazos: IK 3D hacia el manubrio (pose fija)
    for (int side = 0; side < 2; side++) {
        float sx = side == 0 ? 1.0f : -1.0f;
        glm::vec3 S(sx * SHOULDER_REST.x, SHOULDER_REST.y, SHOULDER_REST.z);
        glm::vec3 H(sx * HAND_REST.x, HAND_REST.y, HAND_REST.z);
        glm::vec3 arest = H - S;
        float Ltot = glm::length(arest), L1 = Ltot * 0.5f, L2 = Ltot * 0.5f;
        glm::vec3 Sp = glm::vec3(B0 * glm::vec4(S, 1.0f));
        glm::vec3 G(sx * GRIP_LOCAL.x, GRIP_LOCAL.y, GRIP_LOCAL.z);
        glm::vec3 v = G - Sp;
        float d = glm::length(v);
        if (d > Ltot - 0.002f) { v *= (Ltot - 0.002f) / d; d = Ltot - 0.002f; }
        glm::vec3 dir = v / d;
        glm::vec3 pole = glm::normalize(glm::vec3(sx * 0.4f, -1.0f, 0.0f));
        glm::vec3 pp = pole - dir * glm::dot(pole, dir);
        float a = (L1 * L1 - L2 * L2 + d * d) / (2.0f * d);
        float h = std::sqrt(std::max(L1 * L1 - a * a, 0.0f));
        glm::vec3 K = Sp + dir * a + glm::normalize(pp) * h;
        glm::mat3 R0(B0);
        glm::vec3 u0 = R0 * glm::normalize(arest);
        glm::mat4 Bu = AboutPoint(RotBetween(u0, K - Sp), Sp) * B0;
        glm::vec3 f0 = glm::mat3(Bu) * glm::normalize(arest);
        glm::mat4 Bf = AboutPoint(RotBetween(f0, (Sp + v) - K), K) * Bu;
        out[side == 0 ? UPPER_A : UPPER_B] = Bu;
        out[side == 0 ? FORE_A : FORE_B] = Bf;
    }
}

} // namespace RiderPose
