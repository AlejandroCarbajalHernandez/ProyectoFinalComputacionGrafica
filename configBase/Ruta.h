#pragma once
// Ruta.h - Trazo de la ciclovia (eje) y zonas del recorrido.
// El eje se arma con tramos rectos y curvas (arcos). Cada tramo = { longitud en metros, giro total en grados }.
// Giro positivo = a la izquierda, negativo = a la derecha. 1 unidad de OpenGL = 1 metro.
// Convencion igual a PlayerBike: con rotacion = theta, la bici avanza hacia (-sin(theta), -cos(theta)).
#include <vector>
#include <cmath>
#include <glm/glm.hpp>

// Todo lo de la ruta vive aquí: el recorrido completo, sus tramos y las consultas sobre él.
namespace Ruta {

    // Las tres zonas grandes del recorrido.
    enum Zona { URBANA = 0, RURAL = 1, PUEBLO = 2 };

    // Un pedazo de la ruta: qué tan largo es, cuánto gira y en qué zona está.
    struct Tramo { float largo; float giroDeg; Zona zona; };

    // E1 Polanco 210 m (plana) | E2 Ascenso rural 265 m (sube 26 m) | E3 Tepoztlan 210 m (casi plana)
    // Lista de todos los tramos, desde la salida hasta la meta.
    inline const std::vector<Tramo>& Tramos() {
        static const std::vector<Tramo> t = {
            // E1: ciudad, curvas suaves
            { 60,   0, URBANA },{ 50, -25, URBANA },{ 40,   0, URBANA },{ 60,  30, URBANA },
            // E2: campo, curvas mas cerradas
            { 25,   0, RURAL },{ 45,  40, RURAL },{ 20,   0, RURAL },{ 60, -60, RURAL },
            { 50,  55, RURAL },{ 25,   0, RURAL },{ 40, -30, RURAL },
            // E3: pueblo
            { 30,   0, PUEBLO },{ 50,  35, PUEBLO },{ 30,   0, PUEBLO },{ 60, -35, PUEBLO },{ 40, 0, PUEBLO }
        };
        return t;
    }

    // La ruta se guarda como una lista de puntos muy juntos, uno cada medio metro.
    const float PASO = 0.5f;            // separacion entre muestras del eje (m)
    const float ALTURA_ASCENSO = 26.0f; // metros que sube la zona rural

    // Un punto de la ruta: dónde está, hacia dónde apunta y cuánto se ha recorrido hasta él.
    struct Muestra {
        glm::vec3 pos;     // posicion del eje (x, y, z)
        float headingDeg;  // rotacion Y de la bici en ese punto (grados)
        float s;           // distancia recorrida desde el inicio (m)
        Zona zona;
    };

    // Transición suave entre 0 y 1, sirve para que la subida del terreno no sea brusca.
    inline float SmoothStep(float a, float b, float x) {
        float t = (x - a) / (b - a); if (t < 0) t = 0; if (t > 1) t = 1;
        return t * t * (3.0f - 2.0f * t);
    }

    // Arma la lista de puntos de la ruta recorriendo los tramos uno por uno.
    // Solo se calcula la primera vez; después se reutiliza.
    inline const std::vector<Muestra>& Muestras() {
        static std::vector<Muestra> m;
        if (!m.empty()) return m;
        // Se calcula dónde empieza y termina la zona rural, que es la parte donde el terreno sube.
        float total = 0; for (auto& t : Tramos()) total += t.largo;
        float zIniRural = 0, zFinRural = 0, acum = 0;
        for (auto& t : Tramos()) { if (t.zona == RURAL) { if (zIniRural == 0) zIniRural = acum; zFinRural = acum + t.largo; } acum += t.largo; }

        // Se avanza tramo por tramo con pasos pequeños: en cada paso se guarda un punto y se gira un poco.
        float x = 0, z = 0, th = 0, s = 0;
        for (auto& t : Tramos()) {
            int n = (int)std::ceil(t.largo / PASO);
            float ds = t.largo / n, dth = t.giroDeg / n;
            for (int i = 0; i < n; i++) {
                // La ruta se corta a los 530 m, donde termina el recorrido jugable.
                if (s > 530.0f) break;
                // La altura sube poco a poco a lo largo de la zona rural.
                float y = ALTURA_ASCENSO * SmoothStep(zIniRural, zFinRural, s);
                m.push_back({ glm::vec3(x, y, z), th, s, t.zona });
                // Se avanza hacia donde apunta la ruta y luego se gira un poco para formar la curva.
                float mid = glm::radians(th + 0.5f * dth);
                x += -std::sin(mid) * ds; z += -std::cos(mid) * ds;
                th += dth; s += ds;
            }
            if (s > 530.0f) break;
        }
        ///*m.push_back({ glm::vec3(x, ALTURA_ASCENSO, z), th, s, PUEBLO });*/
        return m;
    }

    // Largo total de la ruta en metros.
    inline float Largo() { return Muestras().back().s; }

    // Punto del eje a la distancia s (interpolado)
    inline Muestra En(float s) {
        const auto& m = Muestras();
        // Antes del inicio o después del final se regresa el primer o el último punto.
        if (s <= 0) return m.front();
        if (s >= m.back().s) return m.back();
        // Se buscan los dos puntos guardados entre los que cae la distancia pedida y se mezclan proporcionalmente.
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
        // Cada punto de la ruta aporta su altura; los más cercanos pesan mucho más que los lejanos.
        float sw = 0.0f, sy = 0.0f;
        for (size_t i = 0; i < m.size(); i += 4) {       // una muestra cada 2 m
            float dx = x - m[i].pos.x, dz = z - m[i].pos.z;
            float d2 = dx * dx + dz * dz;
            // Mientras más lejos está el punto, menos cuenta.
            float w = 1.0f / (d2 * d2 + 1.0f);
            sw += w; sy += w * m[i].pos.y;
        }
        // El resultado es el promedio de esas alturas.
        return sy / sw;
    }

} // namespace Ruta