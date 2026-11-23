#include "PolancoScene.h"
#include "Ruta.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Avance de la bici sobre la ruta (m). main.cpp lo actualiza en cada cuadro.
float PolancoScene::riderS = 0.0f;

// Parámetros geométricos y de control para el cruce vehicular (metro 200)
static const float CRUCE_S = 200.0f;
static const float DIST_ACTIVACION = 60.0f;
static const float ANCHO_CALLE = 12.0f;
static const float LARGO_CRUCE = 120.0f;
static const float ROT_NARIZ = 0.0f;
static const float PISTA_MEDIO = 1.6f;
static const float LINEA_ALTO = PISTA_MEDIO + 1.5f;
static const float CRUCE_Y = 0.105f;
static const float BICI_ZONA = 4.0f;

// Rango en Z reservado para evitar colisión visual con el Museo Soumaya
static const float MUSEO_Z_INI = -20.0f;
static const float MUSEO_Z_FIN = -125.0f;

// Escalas y offsets verticales de edificaciones específicas
static const float EDIF_AB_ESC = 3.0f;
static const float EDIFA_Y = 0.0f;
static const float EDIFB_Y = 0.0f;
static const float CASA1_ESC = 0.75f;
static const float CASA1_Y = -1.15f;

// Puntos de vista predefinidos para alinear la escena urbana con el inicio de la ruta
static const PolancoScene::Viewpoint VIEWPOINTS[PolancoScene::VIEWPOINT_COUNT] = {
    { "Inicio de la ciclovia (Polanco, frente a Museos)", glm::vec3(9.6f, 0.0f, 10.0f), 0.0f },
    { "Frente al Museo Soumaya", glm::vec3(9.6f, 0.0f, -150.0f), 90.0f },
};

// Retorna la configuración del punto de vista solicitado con validación de límites
const PolancoScene::Viewpoint& PolancoScene::GetViewpoint(int index) {
    if (index < 0) index = 0;
    if (index >= VIEWPOINT_COUNT) index = VIEWPOINT_COUNT - 1;
    return VIEWPOINTS[index];
}

// Constructor: inicializa todos los punteros de modelos urbanos y vehículos en nulo
PolancoScene::PolancoScene() {
    museo = nullptr; banca = nullptr; cono = nullptr; puente = nullptr; vias = nullptr;
    arbol = nullptr; luminaria = nullptr; camellon = nullptr;
    autoRojo = nullptr; autoNegro = nullptr; autobus = nullptr;
    edificioA = nullptr; edificioB = nullptr; edificioBeige = nullptr; edificioC = nullptr;
    techoVerde = nullptr; torreCil = nullptr; torreBal = nullptr; torreCrisA = nullptr; torreCrisB = nullptr;
    carretera = nullptr;
    carreteraCruce = nullptr;
    semaforo = nullptr;
    edificio1 = nullptr;
    edificio2 = nullptr;
    casa1 = nullptr;
    casa2 = nullptr;
    locomotora = nullptr; tolva = nullptr; jacaranda = nullptr;
    casaRus1 = nullptr; casaRus2 = nullptr; casaRus3 = nullptr;
}

// Destructor: libera la memoria de todos los modelos 3D y del controlador de semáforo
PolancoScene::~PolancoScene() {
    delete museo;
    delete banca;
    delete cono;
    delete puente;
    delete vias;
    delete arbol;
    delete luminaria;
    delete camellon;
    delete autoRojo;
    delete autoNegro;
    delete autobus;
    delete edificioA;
    delete edificioB;
    delete edificioBeige;
    delete edificioC;
    delete techoVerde;
    delete torreCil;
    delete torreBal;
    delete torreCrisA;
    delete torreCrisB;
    delete carretera;
    delete carreteraCruce;
    delete semaforo;
    delete edificio1;
    delete edificio2;
    delete casa1;
    delete casa2;
    delete locomotora;
    delete tolva;
    delete jacaranda;
    delete casaRus1;
    delete casaRus2;
    delete casaRus3;
}

// Carga los modelos arquitectónicos de Polanco, configura reflejos en cristales y crea el tráfico inicial
void PolancoScene::Init() {
    // Mobiliario urbano, vías y Museo Soumaya
    museo = new Model((char*)"../assets/obj/Polanco/MuseoS.obj");
    banca = new Model((char*)"../assets/obj/Polanco/Banca.obj");
    cono = new Model((char*)"../assets/obj/Polanco/Cono.obj");
    puente = new Model((char*)"../assets/obj/Polanco/Puente.obj");
    vias = new Model((char*)"../assets/obj/Polanco/ViaTren.obj");
    arbol = new Model((char*)"../assets/obj/Polanco/ArbolTex.obj");
    luminaria = new Model((char*)"../assets/obj/Polanco/Luminaria.obj");
    camellon = new Model((char*)"../assets/obj/Polanco/CamellonArbolado.obj");

    // Vehículos para el cruce animado
    autoRojo = new Model((char*)"../assets/obj/Polanco/AutoRojo.obj");
    autoNegro = new Model((char*)"../assets/obj/Polanco/AutoNegro.obj");
    autobus = new Model((char*)"../assets/obj/Polanco/AutobusTuristico.obj");

    // Edificios corporativos y torres residenciales
    edificioA = new Model((char*)"../assets/obj/Polanco/EdificioA.obj");
    edificioB = new Model((char*)"../assets/obj/Polanco/EdificioB.obj");
    edificioBeige = new Model((char*)"../assets/obj/Polanco/EdificioBeige.obj");
    edificioC = new Model((char*)"../assets/obj/Polanco/EdificioC.obj");
    techoVerde = new Model((char*)"../assets/obj/Polanco/EdificioTechoVerde.obj");
    torreCil = new Model((char*)"../assets/obj/Polanco/TorreCilindrica.obj");
    torreBal = new Model((char*)"../assets/obj/Polanco/TorreBalcones.obj");
    torreCrisA = new Model((char*)"../assets/obj/Polanco/TorreCristalA.obj");
    torreCrisB = new Model((char*)"../assets/obj/Polanco/TorreCristalB.obj");

    // Asignación de reflectividad ambiental (envMap) en ventanales de edificios
    edificioA->SetReflectivityMaterial("Vidrio", 0.20f);
    edificioB->SetReflectivityMaterial("Vidrio", 0.20f);
    edificioC->SetReflectivityMaterial("Vidrio", 0.20f);
    edificioBeige->SetReflectivityMaterial("Vidrio", 0.25f);
    techoVerde->SetReflectivityMaterial("Vidrio", 0.20f);
    torreBal->SetReflectivityMaterial("Vidrio", 0.20f);
    torreCil->SetReflectivityMaterial("Vidrio", 0.20f);

    // Vialidades y edificaciones complementarias
    // carretera paralela eliminada (no se carga; el bloque "8. Carretera paralela" no se dibuja)
    carreteraCruce = new Model((char*)"../assets/obj/Polanco/CarreteraCruce.obj");
    edificio1 = new Model((char*)"../assets/obj/Polanco/edificio-01.obj");
    edificio2 = new Model((char*)"../assets/obj/Polanco/edificio-02.obj");
    casa1 = new Model((char*)"../assets/obj/Polanco/Casa_1.obj");
    casa2 = new Model((char*)"../assets/obj/Polanco/Casa_2.obj");

    // Tren estatico (locomotora + 2 tolvas) y jacarandas
    locomotora = new Model((char*)"../assets/obj/Polanco/Locomotora.obj");
    tolva = new Model((char*)"../assets/obj/Polanco/TolvaTren.obj");
    jacaranda = new Model((char*)"../assets/obj/Polanco/Jacaranda.obj");
    // Casas rusticas: transicion ciudad-campo antes del cruce
    casaRus1 = new Model((char*)"../assets/obj/Polanco/CasaRustica1.obj");
    casaRus2 = new Model((char*)"../assets/obj/Polanco/CasaRustica2.obj");
    casaRus3 = new Model((char*)"../assets/obj/Polanco/CasaRustica3.obj");
    locomotora->SetReflectivityMaterial("Vidrio", 0.25f);

    // Inicialización de semáforo desfasado respecto al ciclo de la ciclovía
    semaforo = new Semaforo();
    semaforo->Init();
    semaforo->tRojo = semaforo->tVerde + semaforo->tAmbar + 2.0f;

    // Registro inicial de vehículos en ambos sentidos de circulación
    vehiculos.clear();
    vehiculos.push_back({ autoRojo,  -30.0f, -30.0f, +1.0f, 9.0f,  4.5f });
    vehiculos.push_back({ autobus,   -55.0f, -55.0f, +1.0f, 7.0f, 12.0f });
    vehiculos.push_back({ autoNegro,  30.0f,  30.0f, -1.0f, 9.0f,  4.5f });
    vehiculos.push_back({ autoNegro,  52.0f,  52.0f, -1.0f, 9.0f,  4.5f });
}

// Simula el avance de los vehículos respetando la luz roja, distancia de seguridad y ciclo continuo
void PolancoScene::ActualizarVehiculos(float dt, bool verdeParaCarros) {
    for (size_t i = 0; i < vehiculos.size(); i++) {
        Vehiculo& v = vehiculos[i];
        float nuevo = v.u + v.dir * v.vel * dt;

        // Detención antes de la línea de alto si el semáforo no está en verde
        float paro = -v.dir * (LINEA_ALTO + v.largo * 0.5f);
        bool cruzariaLinea = (v.dir > 0.0f) ? (v.u <= paro && nuevo > paro) : (v.u >= paro && nuevo < paro);
        if (cruzariaLinea && !verdeParaCarros) nuevo = paro;

        // Prevención de colisiones manteniendo separación mínima con el vehículo delantero
        for (size_t j = 0; j < vehiculos.size(); j++) {
            if (j == i || vehiculos[j].dir != v.dir) continue;
            float adelante = (vehiculos[j].u - nuevo) * v.dir;
            float minimo = 0.5f * (v.largo + vehiculos[j].largo) + 2.0f;
            if (adelante > 0.0f && adelante < minimo) { nuevo = v.u; break; }
        }
        v.u = nuevo;

        // Reinicio cíclico al salir de los extremos del cruce
        const float U = LARGO_CRUCE * 0.5f;
        if (v.dir > 0.0f && v.u > U) v.u = -U;
        else if (v.dir < 0.0f && v.u < -U) v.u = U;
    }
}

// Renderiza toda la zona urbana de Polanco alineada dinámicamente con la curvatura de Ruta
void PolancoScene::Draw(Shader& shader, GLint modelLoc) {
    Ruta::Muestra mInicio = Ruta::En(0.0f);
    glm::mat4 baseAlineada = worldMatrix;
    baseAlineada = glm::translate(baseAlineada, mInicio.pos);
    baseAlineada = glm::rotate(baseAlineada, glm::radians(mInicio.headingDeg), glm::vec3(0.0f, 1.0f, 0.0f));

    // Calcula la matriz de transformación local siguiendo el eje longitudinal de la ruta
    auto Base = [&](float z) {
        if (z > 0.0f) return glm::translate(baseAlineada, glm::vec3(0.0f, 0.0f, z));
        Ruta::Muestra m = Ruta::En(-z);
        glm::mat4 b = worldMatrix;
        b = glm::translate(b, m.pos);
        b = glm::rotate(b, glm::radians(m.headingDeg), glm::vec3(0.0f, 1.0f, 0.0f));
        return b;
        };

    glm::mat4 model(1.0f);

    // 1. Museo Soumaya (con alta reflectividad metálica en su fachada)
    if (museo) {
        model = Base(-70.0f);
        model = glm::translate(model, glm::vec3(-38.0f, -0.7f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(glGetUniformLocation(shader.Program, "reflectivity"), 0.55f);
        museo->Draw(shader);
        glUniform1f(glGetUniformLocation(shader.Program, "reflectivity"), 0.0f);
    }

    // 2. Vías y Camellón
    if (vias) {
        for (float z = 5.0f; z > -160.0f; z -= 30.0f) {
            model = Base(z);
            model = glm::translate(model, glm::vec3(-6.0f, 0.02f, 0.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            vias->Draw(shader);
        }
    }
    if (camellon) {
        model = Base(-60.0f);
        model = glm::translate(model, glm::vec3(-3.5f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        camellon->Draw(shader);
    }

    // 3. Edificios, torres y casas.
    // Acomodo calculado con las medidas reales de cada modelo: ningun edificio se traslapa con otro,
    // todos quedan a >= 4 m de separacion y fuera del Museo y de la calle transversal del cruce.
    // Para mover uno: "s" es la distancia sobre la ruta (m), "x" el desplazamiento lateral
    // (negativo = izquierda, positivo = derecha) y "rotY" el giro en grados.
    struct Colocado { Model* m; float s, x, rotY, esc, y, refl; };
    const Colocado edificios[] = {
        // --- Lado derecho, fila 1 (frente a la calle) ---
        { edificioBeige,     6.0f,   40.2f, -90.0f, 1.0f, 0.0f, 0.0f },
        { torreCil,         44.0f,   38.4f, -90.0f, 1.0f, 0.0f, 0.0f },
        { edificioC,        76.5f,   32.2f, -90.0f, 1.0f, 0.0f, 0.0f },
        { torreBal,        113.0f,   36.6f, -90.0f, 1.0f, 0.0f, 0.0f },
        // --- Lado derecho, fila 2 (torres al fondo) ---
        { torreCrisA,       22.0f,   72.4f, -90.0f, 1.0f, 0.0f, 0.22f },
        { torreBal,         53.0f,   71.9f, -90.0f, 1.0f, 0.0f, 0.0f },
        { edificioB,       105.0f,   67.5f, -90.0f, 1.0f, 0.0f, 0.0f },
        { torreCil,        133.0f,   73.7f, -90.0f, 1.0f, 0.0f, 0.0f },
        // --- Lado derecho, fila 3 (skyline al fondo) ---
        { torreBal,         10.0f,  102.9f, -90.0f, 1.0f, 0.0f, 0.0f },
        { torreCrisA,       41.0f,  103.4f, -90.0f, 1.0f, 0.0f, 0.22f },
        { torreCil,        124.5f,  104.7f, -90.0f, 1.0f, 0.0f, 0.0f },
        // --- Lado izquierdo, fila 1 (frente a la calle) ---
        { edificioC,        15.5f,  -17.2f,  90.0f, 1.0f, 0.0f, 0.0f },
        { edificioB,       106.5f,  -17.2f,  90.0f, 1.0f, 0.0f, 0.0f },
        { edificioA,       130.0f,  -18.2f,  90.0f, 1.0f, 0.0f, 0.0f },
        // --- Lado izquierdo, fila 2 (al fondo); el edificio blanco (techoVerde) esta detras de la salida ---
        { edificioB,        20.5f,  -43.2f,  90.0f, 1.0f, 0.0f, 0.0f },
        { techoVerde,      -27.5f,    0.5f, 180.0f, 0.75f, 0.0f, 0.0f },   // edificio blanco: detras de la salida; 180 = ventanas hacia la ruta; escala 0.75 para dejar 3 m de aire
        // --- Casas junto al cruce ---
        { casa1,           179.0f,  -15.2f,   0.0f, CASA1_ESC, CASA1_Y, 0.0f },
        { casa2,           182.0f,   44.0f, -90.0f, 0.62f, 0.0f, 0.0f },   // casa grande, mas pequena y detras de la carretera
        // --- Casas rusticas (transicion ciudad a campo, antes del cruce); el frente mira a la ruta ---
        { casaRus2,   148.0f,   -16.0f,   90.0f, 1.0f, 0.0f, 0.0f },
        { casaRus3,   150.0f,   -26.0f,   90.0f, 1.0f, 0.0f, 0.0f },
        { casaRus1,   160.0f,   -16.0f,   90.0f, 1.0f, 0.0f, 0.0f },
        { casaRus1,   168.0f,   -26.0f,   90.0f, 1.0f, 0.0f, 0.0f },
        { casaRus3,   146.0f,    38.0f,  -90.0f, 1.0f, 0.0f, 0.0f },
        { casaRus3,   150.0f,    50.0f,  -90.0f, 1.0f, 0.0f, 0.0f },
        { casaRus2,   155.0f,    38.0f,  -90.0f, 1.0f, 0.0f, 0.0f },
        { casaRus1,   158.0f,    50.0f,  -90.0f, 1.0f, 0.0f, 0.0f },
        { casaRus1,   163.0f,    38.0f,  -90.0f, 1.0f, 0.0f, 0.0f },
        { casaRus2,   165.0f,    50.0f,  -90.0f, 1.0f, 0.0f, 0.0f },
        { casaRus2,   171.0f,    38.0f,  -90.0f, 1.0f, 0.0f, 0.0f },
    };

    const GLint locRefl = glGetUniformLocation(shader.Program, "reflectivity");
    for (const Colocado& e : edificios) {
        if (!e.m) continue;
        model = Base(-e.s);
        model = glm::translate(model, glm::vec3(e.x, e.y, 0.0f));
        model = glm::rotate(model, glm::radians(e.rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        if (e.esc != 1.0f) model = glm::scale(model, glm::vec3(e.esc));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform1f(locRefl, e.refl);
        e.m->Draw(shader);
    }
    glUniform1f(locRefl, 0.0f);

    // 6. Edificios z > 0 (tramo previo)
    if (edificio1) {
        for (float z = 45.0f; z <= 95.0f; z += 25.0f) {
            model = Base(z);
            model = glm::translate(model, glm::vec3(-24.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            edificio1->Draw(shader);
        }
    }

    if (edificio2) {
        for (float z = 45.0f; z <= 95.0f; z += 25.0f) {
            model = Base(z);
            model = glm::translate(model, glm::vec3(43.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            edificio2->Draw(shader);
        }
    }

    if (torreCrisB) {
        for (float z = 55.0f; z <= 105.0f; z += 50.0f) {
            model = Base(z);
            model = glm::translate(model, glm::vec3(-55.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            torreCrisB->Draw(shader);
        }
    }

    if (torreCrisA) {
        for (float z = 55.0f; z <= 105.0f; z += 50.0f) {
            model = Base(z);
            model = glm::translate(model, glm::vec3(68.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            torreCrisA->Draw(shader);
        }
    }

    if (torreBal) {
        model = Base(125.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        torreBal->Draw(shader);
    }

    // 7. Árboles y luminarias
    if (arbol) {
        for (float z = 5.0f; z > -192.0f; z -= 12.0f) {
            model = Base(z); model = glm::translate(model, glm::vec3(16.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); arbol->Draw(shader);

            model = Base(z - 5.0f); model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); arbol->Draw(shader);
        }
    }

    if (luminaria) {
        for (float z = 0.0f; z > -190.0f; z -= 22.0f) {
            model = Base(z); model = glm::translate(model, glm::vec3(19.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); luminaria->Draw(shader);
        }
    }

    // 7b. Tren estatico sobre las vias (frente a la ruta, junto al Museo Soumaya).
    // Las vias estan en x = -6. "s" = distancia sobre la ruta (m); el frente de la locomotora mira hacia el inicio.
    {
        const float TREN_X = -6.0f;     // lateral: sobre las vias
        const float TREN_Y = 0.25f;     // altura de las ruedas sobre el piso (subir/bajar si flota o se hunde)
        const float TREN_S = 118.0f;    // centro de la locomotora; las tolvas siguen detras
        struct Vagon { Model* m; float ds; };
        const Vagon tren[] = { { locomotora, 0.0f },{ tolva, 16.0f },{ tolva, 30.0f } };
        for (const Vagon& v : tren) {
            if (!v.m) continue;
            model = Base(-(TREN_S + v.ds));
            model = glm::translate(model, glm::vec3(TREN_X, TREN_Y, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform1f(locRefl, 0.0f);
            v.m->Draw(shader);
        }
    }

    // 7c. Jacarandas intercaladas entre los arboles de la ruta.
    // Los arboles normales estan en x = 16 (s = -5, 7, 19, ...) y x = -1 (s = 0, 12, 24, ...);
    // cada jacaranda va en el hueco entre dos de ellos. Giro y escala distintos para que no se repitan.
    if (jacaranda) {
        auto Plantar = [&](float s, float x) {
            const float azar = std::fmod(std::fabs(std::sin(s * 12.9898f + x * 78.233f)) * 43758.5453f, 1.0f);
            model = Base(-s);
            model = glm::translate(model, glm::vec3(x, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(azar * 360.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.85f + azar * 0.35f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            jacaranda->Draw(shader);
            };
        for (float s = 1.0f; s < 191.0f; s += 12.0f) Plantar(s, 16.0f);          // hilera derecha: en cada hueco, hasta el cruce
        for (float s = 18.0f; s < 191.0f; s += 24.0f)                            // hilera junto a la ruta: un hueco si, uno no
            if (s < 108.0f || s > 158.0f) Plantar(s, -1.0f);                      // sin tapar el tren (s 110 a 155)
    }

    // 8. Carretera paralela
    if (carretera) {
        for (float z = 10.0f; z > -192.0f; z -= 10.0f) {
            model = Base(z);
            model = glm::translate(model, glm::vec3(27.0f, 0.01f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            carretera->Draw(shader);
        }
    }

    // 9. Mobiliario
    if (banca) {
        model = Base(-20.0f); model = glm::translate(model, glm::vec3(2.5f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); banca->Draw(shader);
    }
    if (cono) {
        model = Base(-25.0f); model = glm::translate(model, glm::vec3(5.5f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); cono->Draw(shader);
    }

    // 10. Semáforos urbanos fijos
    if (semaforo) {
        const float t = static_cast<float>(glfwGetTime());
        const float SEM_X = 21.0f;

        glm::mat4 m = Base(-45.0f);
        m = glm::translate(m, glm::vec3(SEM_X, 0.0f, 0.0f));
        semaforo->Draw(shader, modelLoc, m, t, 0.0f);

        m = Base(-95.0f);
        m = glm::translate(m, glm::vec3(SEM_X, 0.0f, 0.0f));
        semaforo->Draw(shader, modelLoc, m, t, semaforo->tVerde + semaforo->tAmbar);

        // Copia del semaforo 2, mas cerca de la ruta para que se vea (misma fase que el original)
        m = Base(-95.0f);
        m = glm::translate(m, glm::vec3(4.5f, 0.0f, 0.0f));
        semaforo->Draw(shader, modelLoc, m, t, semaforo->tVerde + semaforo->tAmbar);
    }

    // 11. Cruce animado con vehículos (se activa al acercarse el ciclista al metro 200)
    {
        const float t = static_cast<float>(glfwGetTime());
        float dt = t - ultimoT;
        ultimoT = t;
        if (dt > 0.1f) dt = 0.1f;
        if (dt < 0.0f) dt = 0.0f;

        // Activación y reinicio del ciclo de tráfico según proximidad del jugador
        if (cruceT0 < 0.0f && riderS >= CRUCE_S - DIST_ACTIVACION) {
            cruceT0 = t;
        }
        else if (cruceT0 >= 0.0f && riderS < CRUCE_S - DIST_ACTIVACION - 20.0f) {
            cruceT0 = -1.0f;
            for (size_t i = 0; i < vehiculos.size(); i++) vehiculos[i].u = vehiculos[i].u0;
        }
        const bool activo = (cruceT0 >= 0.0f);
        const float tCiclo = activo ? (t - cruceT0) : 0.0f;

        const float desfaseCarros = semaforo ? semaforo->tVerde + semaforo->tAmbar : 0.0f;
        Semaforo::Fase faseCarros = semaforo ? semaforo->FaseEn(tCiclo, desfaseCarros) : Semaforo::ROJO;

        // Actualización de tráfico cediento el paso si el ciclista está dentro de la zona de cruce
        const bool biciEnCruce = std::fabs(riderS - CRUCE_S) < BICI_ZONA;
        if (activo) ActualizarVehiculos(dt, faseCarros == Semaforo::VERDE && !biciEnCruce);

        // Renderizado de la intersección vial, vehículos y semáforos sincronizados
        if (carreteraCruce) {
            model = Base(-CRUCE_S);
            model = glm::translate(model, glm::vec3(14.0f, CRUCE_Y, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            carreteraCruce->Draw(shader);
        }

        for (size_t i = 0; i < vehiculos.size(); i++) {
            const Vehiculo& v = vehiculos[i];
            if (!v.modelo) continue;
            model = Base(-CRUCE_S);
            model = glm::translate(model, glm::vec3(v.u + 8.0f, CRUCE_Y + 0.01f, v.dir * ANCHO_CALLE * 0.25f));
            model = glm::rotate(model, glm::radians(ROT_NARIZ + (v.dir > 0.0f ? 0.0f : 180.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            v.modelo->Draw(shader);
        }

        if (semaforo) {
            glm::mat4 m = Base(-CRUCE_S);
            m = glm::translate(m, glm::vec3(PISTA_MEDIO + 0.6f, CRUCE_Y, ANCHO_CALLE * 0.5f + 1.5f));
            semaforo->Draw(shader, modelLoc, m, tCiclo, 0.0f);

            m = Base(-CRUCE_S);
            m = glm::translate(m, glm::vec3(-(LINEA_ALTO + 1.0f), CRUCE_Y, ANCHO_CALLE * 0.5f + 1.0f));
            m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            semaforo->Draw(shader, modelLoc, m, tCiclo, desfaseCarros);

            m = Base(-CRUCE_S);
            m = glm::translate(m, glm::vec3(LINEA_ALTO + 1.0f, CRUCE_Y, -(ANCHO_CALLE * 0.5f + 1.0f)));
            m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            semaforo->Draw(shader, modelLoc, m, tCiclo, desfaseCarros);
        }
    }
}