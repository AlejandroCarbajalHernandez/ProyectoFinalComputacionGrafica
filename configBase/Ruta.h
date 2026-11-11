#pragma once
// Ruta.h - Trazo de la ciclovia (eje) y zonas del recorrido.
// El eje se arma con tramos rectos y curvas (arcos). Cada tramo = { longitud en metros, giro total en grados }.
// Giro positivo = a la izquierda, negativo = a la derecha. 1 unidad de OpenGL = 1 metro.
// Convencion igual a PlayerBike: con rotacion = theta, la bici avanza hacia (-sin(theta), -cos(theta)).
#include <vector>
#include <cmath>
#include <glm/glm.hpp>

namespace Ruta {

enum Zona { URBANA = 0, RURAL = 1, PUEBLO = 2 };

struct Tramo { float largo; float giroDeg; Zona zona; };

// E1 Polanco 210 m (plana) | E2 Ascenso rural 265 m (sube 26 m) | E3 Tepoztlan 210 m (casi plana)
inline const std::vector<Tramo>& Tramos() {
    static const std::vector<Tramo> t = {
        // E1: ciudad, curvas suaves
        {  60,   0, URBANA }, { 50, -25, URBANA }, { 40,   0, URBANA }, { 60,  30, URBANA },
        // E2: campo, curvas mas cerradas
        {  25,   0, RURAL  }, { 45,  40, RURAL  }, { 20,   0, RURAL  }, { 60, -60, RURAL  },
        {  50,  55, RURAL  }, { 25,   0, RURAL  }, { 40, -30, RURAL  },
        // E3: pueblo
        {  30,   0, PUEBLO }, { 50,  35, PUEBLO }, { 30,   0, PUEBLO }, { 60, -35, PUEBLO }, { 40, 0, PUEBLO }
    };
    return t;
}

const float PASO = 0.5f;            // separacion entre muestras del eje (m)
const float ALTURA_ASCENSO = 26.0f; // metros que sube la zona rural

struct Muestra {
    glm::vec3 pos;     // posicion del eje (x, y, z)
    float headingDeg;  // rotacion Y de la bici en ese punto (grados)
    float s;           // distancia recorrida desde el inicio (m)
    Zona zona;
};

inline float SmoothStep(float a, float b, float x) {
    float t = (x - a) / (b - a); if (t < 0) t = 0; if (t > 1) t = 1;
    return t * t * (3.0f - 2.0f * t);
}

inline const std::vector<Muestra>& Muestras() {
    static std::vector<Muestra> m;
    if (!m.empty()) return m;
    float total = 0; for (auto& t : Tramos()) total += t.largo;
    float zIniRural = 0, zFinRural = 0, acum = 0;
    for (auto& t : Tramos()) { if (t.zona == RURAL) { if (zIniRural == 0) zIniRural = acum; zFinRural = acum + t.largo; } acum += t.largo; }

    float x = 0, z = 0, th = 0, s = 0;
    for (auto& t : Tramos()) {
        int n = (int)std::ceil(t.largo / PASO);
        float ds = t.largo / n, dth = t.giroDeg / n;
        for (int i = 0; i < n; i++) {
            if (s > 530.0f) break;
            float y = ALTURA_ASCENSO * SmoothStep(zIniRural, zFinRural, s);
            m.push_back({ glm::vec3(x, y, z), th, s, t.zona });
            float mid = glm::radians(th + 0.5f * dth);
            x += -std::sin(mid) * ds; z += -std::cos(mid) * ds;
            th += dth; s += ds;
        }
        if (s > 530.0f) break;
    }
    ///*m.push_back({ glm::vec3(x, ALTURA_ASCENSO, z), th, s, PUEBLO });*/
    return m;
}

inline float Largo() { return Muestras().back().s; }

// Punto del eje a la distancia s (interpolado)
inline Muestra En(float s) {
    const auto& m = Muestras();
    if (s <= 0) return m.front();
    if (s >= m.back().s) return m.back();
    int i = (int)(s / PASO); if (i > (int)m.size() - 2) i = (int)m.size() - 2;
    float f = (s - m[i].s) / (m[i + 1].s - m[i].s);
    Muestra r = m[i];
    r.pos = m[i].pos + (m[i + 1].pos - m[i].pos) * f;
    r.headingDeg = m[i].headingDeg + (m[i + 1].headingDeg - m[i].headingDeg) * f;
    r.s = s;
    return r;
}

// Altura del suelo en (x, z): promedio de la altura del eje, con mas peso a los puntos cercanos.
// Sobre la pista da la altura de la pista; lejos de ella cambia de forma suave.
inline float AlturaSuelo(float x, float z) {
    const auto& m = Muestras();
    float sw = 0.0f, sy = 0.0f;
    for (size_t i = 0; i < m.size(); i += 4) {       // una muestra cada 2 m
        float dx = x - m[i].pos.x, dz = z - m[i].pos.z;
        float d2 = dx * dx + dz * dz;
        float w = 1.0f / (d2 * d2 + 1.0f);
        sw += w; sy += w * m[i].pos.y;
    }
    return sy / sw;
}

} // namespace Ruta
