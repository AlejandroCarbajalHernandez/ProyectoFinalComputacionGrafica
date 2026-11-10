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

// ---- Cruce de carretera (ajusta estos valores viendo la escena) ----
static const float CRUCE_S = 200.0f;  // distancia sobre la ruta donde esta el cruce
static const float DIST_ACTIVACION = 60.0f;   // la animacion arranca cuando faltan estos metros
static const float ANCHO_CALLE = 12.0f;   // ancho de CarreteraCruce.obj (un carril por sentido)
static const float LARGO_CRUCE = 120.0f;  // largo de CarreteraCruce.obj
static const float ROT_NARIZ = 0.0f;    // pon 180 si los autos van de reversa
static const float PISTA_MEDIO = 1.6f;                  // 1.25 de asfalto + 0.18 de bordillo + holgura
static const float LINEA_ALTO = PISTA_MEDIO + 1.5f;    // 3.1 m
static const float CRUCE_Y = 0.105f;                // justo arriba de los bordillos (0.10)
static const float BICI_ZONA = 4.0f;                  // m a cada lado del cruce donde la bici bloquea a los carros

// ---- Hueco libre del Museo Soumaya (museo en z = -70, x = -38) ----
// Ningun edificio del lado izquierdo se dibuja con z entre estos dos valores.
// Si quieres mas espacio, amplia el rango (ej. -20 y -120).
static const float MUSEO_Z_INI = -20.0f;
static const float MUSEO_Z_FIN = -125.0f;

// ---- Escala y altura (Y) de los edificios grandes y las casas ----
// Al escalar, el modelo crece desde su origen: si el origen no esta en la base, el modelo "vuela".
// Si flota -> numero mas negativo. Si se hunde en el piso -> numero menos negativo / positivo.
static const float EDIF_AB_ESC = 3.0f;   // escala de edificioA y edificioB (antes 2.0)
static const float EDIFA_Y = 0.0f;       // altura de edificioA
static const float EDIFB_Y = 0.0f;       // altura de edificioB
static const float CASA1_ESC = 1.5f;     // escala de casa1
static const float CASA1_Y = -2.0f;      // altura de casa1 (negativo = bajarla al piso)

static const PolancoScene::Viewpoint VIEWPOINTS[PolancoScene::VIEWPOINT_COUNT] = {
    { "Inicio de la ciclovia (Polanco, frente a Museos)", glm::vec3(9.6f, 0.0f, 10.0f), 0.0f },
    { "Frente al Museo Soumaya", glm::vec3(9.6f, 0.0f, -150.0f), 90.0f },
};

const PolancoScene::Viewpoint& PolancoScene::GetViewpoint(int index) {
    if (index < 0) index = 0;
    if (index >= VIEWPOINT_COUNT) index = VIEWPOINT_COUNT - 1;
    return VIEWPOINTS[index];
}

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

// Puedes agregar más según los nombres de los archivos que tengas
}

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

}

void PolancoScene::Init() {
    museo = new Model((char*)"../assets/obj/Polanco/MuseoS.obj");
    banca = new Model((char*)"../assets/obj/Polanco/Banca.obj");
    cono = new Model((char*)"../assets/obj/Polanco/Cono.obj");
    puente = new Model((char*)"../assets/obj/Polanco/Puente.obj");
    vias = new Model((char*)"../assets/obj/Polanco/ViaTren.obj");
    arbol = new Model((char*)"../assets/obj/Polanco/Arbol.obj");
    luminaria = new Model((char*)"../assets/obj/Polanco/Luminaria.obj");
    camellon = new Model((char*)"../assets/obj/Polanco/CamellonArbolado.obj");

    autoRojo = new Model((char*)"../assets/obj/Polanco/AutoRojo.obj");
    autoNegro = new Model((char*)"../assets/obj/Polanco/AutoNegro.obj");
    autobus = new Model((char*)"../assets/obj/Polanco/AutobusTuristico.obj");

    edificioA = new Model((char*)"../assets/obj/Polanco/EdificioA.obj");
    edificioB = new Model((char*)"../assets/obj/Polanco/EdificioB.obj");
    edificioBeige = new Model((char*)"../assets/obj/Polanco/EdificioBeige.obj");
    edificioC = new Model((char*)"../assets/obj/Polanco/EdificioC.obj");
    techoVerde = new Model((char*)"../assets/obj/Polanco/EdificioTechoVerde.obj");
    torreCil = new Model((char*)"../assets/obj/Polanco/TorreCilindrica.obj");
    torreBal = new Model((char*)"../assets/obj/Polanco/TorreBalcones.obj");
    torreCrisA = new Model((char*)"../assets/obj/Polanco/TorreCristalA.obj");
    torreCrisB = new Model((char*)"../assets/obj/Polanco/TorreCristalB.obj");

    carretera = new Model((char*)"../assets/obj/Polanco/Carretera.obj");
    carreteraCruce = new Model((char*)"../assets/obj/Polanco/CarreteraCruce.obj");   // la que cruza la ciclovia
    edificio1 = new Model((char*)"../assets/obj/Polanco/edificio-01.obj");
    edificio2 = new Model((char*)"../assets/obj/Polanco/edificio-02.obj");
    casa1 = new Model((char*)"../assets/obj/Polanco/Casa_1.obj"); // Si tienes Casa_1 o ajusta al nombre real
    casa2 = new Model((char*)"../assets/obj/Polanco/Casa_2.obj");
    // Semaforos (si ya los creas en otro lado, quita estas tres lineas)
    semaforo = new Semaforo();
    semaforo->Init();
    semaforo->tRojo = semaforo->tVerde + semaforo->tAmbar + 2.0f;   // 2 s con todo en rojo entre cambios

    // Trafico del cruce: u = posicion a lo largo de la carretera (0 = eje de la ciclovia)
    // dir = +1 hacia +X, -1 hacia -X
    vehiculos.clear();
    vehiculos.push_back({ autoRojo,  -30.0f, -30.0f, +1.0f, 9.0f,  4.5f });
    vehiculos.push_back({ autobus,   -55.0f, -55.0f, +1.0f, 7.0f, 12.0f });
    vehiculos.push_back({ autoNegro,  30.0f,  30.0f, -1.0f, 9.0f,  4.5f });
    vehiculos.push_back({ autoNegro,  52.0f,  52.0f, -1.0f, 9.0f,  4.5f });
}

// Mueve los vehiculos del cruce. Solo avanzan con verde; los que ya entraron al cruce terminan de pasar.
void PolancoScene::ActualizarVehiculos(float dt, bool verdeParaCarros) {
    for (size_t i = 0; i < vehiculos.size(); i++) {
        Vehiculo& v = vehiculos[i];
        float nuevo = v.u + v.dir * v.vel * dt;

        // Linea de alto: si no hay verde, no pasa de ahi
        float paro = -v.dir * (LINEA_ALTO + v.largo * 0.5f);
        bool cruzariaLinea = (v.dir > 0.0f) ? (v.u <= paro && nuevo > paro) : (v.u >= paro && nuevo < paro);
        if (cruzariaLinea && !verdeParaCarros) nuevo = paro;

        // Distancia minima con el vehiculo de adelante (mismo sentido)
        for (size_t j = 0; j < vehiculos.size(); j++) {
            if (j == i || vehiculos[j].dir != v.dir) continue;
            float adelante = (vehiculos[j].u - nuevo) * v.dir;
            float minimo = 0.5f * (v.largo + vehiculos[j].largo) + 2.0f;
            if (adelante > 0.0f && adelante < minimo) { nuevo = v.u; break; }
        }
        v.u = nuevo;

        // Al salir por un extremo reaparece en el otro
        const float U = LARGO_CRUCE * 0.5f;
        if (v.dir > 0.0f && v.u > U) v.u = -U;
        else if (v.dir < 0.0f && v.u < -U) v.u = U;
    }
}

void PolancoScene::Draw(Shader& shader, GLint modelLoc) {
    // Sincronización exacta con el inicio de la ruta principal (s = 0)
    Ruta::Muestra mInicio = Ruta::En(0.0f);
    glm::mat4 baseAlineada = worldMatrix;
    baseAlineada = glm::translate(baseAlineada, mInicio.pos);
    baseAlineada = glm::rotate(baseAlineada, glm::radians(mInicio.headingDeg), glm::vec3(0.0f, 1.0f, 0.0f));

    // Base(z): sistema local sobre el eje de la ruta a la distancia s = -z.
    // Asi todo acompaña las curvas de la ciclovia y nada la atraviesa.
    // (Para z > 0, antes del inicio de la ruta, se avanza en recta como antes.)
    auto Base = [&](float z) {
        if (z > 0.0f) return glm::translate(baseAlineada, glm::vec3(0.0f, 0.0f, z));
        Ruta::Muestra m = Ruta::En(-z);
        glm::mat4 b = worldMatrix;
        b = glm::translate(b, m.pos);
        b = glm::rotate(b, glm::radians(m.headingDeg), glm::vec3(0.0f, 1.0f, 0.0f));
        return b;
        };

    // true si z cae en el hueco que se deja libre para que luzca el museo
    auto EnMuseo = [&](float z) {
        return (z < MUSEO_Z_INI && z > MUSEO_Z_FIN);
        };

    glm::mat4 model(1.0f);

    // ==========================================
    // 1. MUSEO SOUMAYA (lejos de la pista, a la izquierda, a media ruta)
    // ==========================================
    if (museo) {
        model = Base(-70.0f);
        model = glm::translate(model, glm::vec3(-38.0f, -0.7f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        museo->Draw(shader);
    }

    // ==========================================
    // 2. VÍAS DEL TREN Y CAMELLÓN (Paralelos a la ciclovía)
    // ==========================================
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
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // rotado 90
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        camellon->Draw(shader);
    }

    // ==========================================
    // 3. PUENTE PEATONAL (sobre la carretera, no sobre la pista)
    // ==========================================
   /* if (puente) {
        model = Base(-50.0f);
        model = glm::translate(model, glm::vec3(32.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.8f, 1.6f, 0.8f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        puente->Draw(shader);
    }*/

    // ==========================================
    // 4. EDIFICIOS LADO IZQUIERDO
    //    Ninguno entre z = -20 y z = -125 (hueco del museo, ver MUSEO_Z_INI / MUSEO_Z_FIN)
    // ==========================================
    if (edificioA) {
        for (float z = 20.0f; z >= -160.0f; z -= 170.0f) { // z = 20 y -150 (antes y despues del museo)
            model = Base(z);
            model = glm::translate(model, glm::vec3(-20.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            edificioA->Draw(shader);
        }
    }

    if (edificioB) {
        for (float z = 15.0f; z >= -160.0f; z -= 165.0f) { // z = 15 y -150
            model = Base(z);
            model = glm::translate(model, glm::vec3(-60.0f, 0.0f, 0.0f)); // Segunda línea exterior izquierda
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            edificioB->Draw(shader);
        }
    }

    if (edificioC) {
        model = Base(-183.0f); model = glm::translate(model, glm::vec3(-20.0f, 0.0f, 0.0f)); // alejado 2 m del cruce
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); edificioC->Draw(shader);
    }

    if (techoVerde) {
        model = Base(-130.0f); model = glm::translate(model, glm::vec3(-55.0f, 0.0f, 0.0f)); // Justo despues del museo
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); techoVerde->Draw(shader);
    }

    if (torreCrisB) {
        for (float z = 40.0f; z >= -190.0f; z -= 65.0f) { // z = 40, [-25 -90 omitidos], -155
            if (EnMuseo(z)) continue;
            model = Base(z);
            model = glm::translate(model, glm::vec3(-85.0f, 0.0f, 0.0f)); // Fondo profundo izquierdo
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            torreCrisB->Draw(shader);
        }
    }

    // ==========================================
    // 5. EDIFICIOS LADO DERECHO (más allá de la carretera, x >= 40)
    // ==========================================
    if (torreBal) {
        for (float z = 5.0f; z >= -140.0f; z -= 35.0f) {
            model = Base(z);
            model = glm::translate(model, glm::vec3(40.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            torreBal->Draw(shader);
        }
    }

    if (torreCil) {
        for (float z = -10.0f; z >= -150.0f; z -= 50.0f) {
            model = Base(z);
            model = glm::translate(model, glm::vec3(60.0f, 0.0f, 0.0f)); // Segunda línea exterior derecha
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            torreCil->Draw(shader);
        }
    }

    if (edificioBeige) {
        for (float z = -15.0f; z >= -140.0f; z -= 45.0f) {
            model = Base(z);
            model = glm::translate(model, glm::vec3(40.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            edificioBeige->Draw(shader);
        }
    }

    if (torreCrisA) {
        for (float z = -30.0f; z >= -160.0f; z -= 50.0f) {
            model = Base(z);
            model = glm::translate(model, glm::vec3(60.0f, 0.0f, 0.0f)); // Fondo profundo derecho
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            torreCrisA->Draw(shader);
        }
    }

    // ==========================================
    // 5b. UNA CASA DE CADA LADO DE LA RUTA (cerca del cruce, s = 200)
    // ==========================================
    if (casa1) {
        model = Base(-180.0f);
        model = glm::translate(model, glm::vec3(-45.0f, CASA1_Y, 0.0f)); // izquierda
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(CASA1_ESC));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        casa1->Draw(shader);
    }

    if (casa2) {
        model = Base(-180.0f);
        model = glm::translate(model, glm::vec3(45.0f, 0.0f, 0.0f)); // derecha
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.8f));                  // 0.8 = 80% del tamaño original
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        casa2->Draw(shader);
    }

    // ==========================================
    // 5c. EDIFICIOS ANTES DEL INICIO DE LA RUTA (z > 0, tramo recto)
    //     Una calle ordenada a cada lado, cada 25 m, sin tocar los demas edificios.
    // ==========================================
    if (edificio1) {
        for (float z = 45.0f; z <= 95.0f; z += 25.0f) { // z = 45, 70, 95
            model = Base(z);
            model = glm::translate(model, glm::vec3(-24.0f, 0.0f, 0.0f)); // Lado izquierdo
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            edificio1->Draw(shader);
        }
    }

    if (edificio2) {
        for (float z = 45.0f; z <= 95.0f; z += 25.0f) { // z = 45, 70, 95
            model = Base(z);
            model = glm::translate(model, glm::vec3(43.0f, 0.0f, 0.0f)); // Lado derecho, mas alla de la carretera
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            edificio2->Draw(shader);
        }
    }

    if (torreCrisB) {
        for (float z = 55.0f; z <= 105.0f; z += 50.0f) { // z = 55, 105
            model = Base(z);
            model = glm::translate(model, glm::vec3(-55.0f, 0.0f, 0.0f)); // Segunda fila izquierda
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            torreCrisB->Draw(shader);
        }
    }

    if (torreCrisA) {
        for (float z = 55.0f; z <= 105.0f; z += 50.0f) { // z = 55, 105
            model = Base(z);
            model = glm::translate(model, glm::vec3(68.0f, 0.0f, 0.0f)); // Segunda fila derecha
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            torreCrisA->Draw(shader);
        }
    }

    // Torre con balcones en el centro del inicio (sobre el eje, cerrando la calle al fondo)
    if (torreBal) {
        model = Base(125.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // x = 0: centro de la ruta
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // prueba 180 si mira al reves
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        torreBal->Draw(shader);
    }

    // Edificio verde enfrente de la torre (del lado de la ruta)
    if (techoVerde) {
        model = Base(100.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // x = 0: alineado con la torre
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // prueba 180 si mira al reves
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        techoVerde->Draw(shader);
    }

    // ==========================================
    // 6. ÁRBOLES Y LUMINARIAS (A lo largo de los flancos)
    // ==========================================
    if (arbol) {
        for (float z = 5.0f; z > -150.0f; z -= 12.0f) {
            model = Base(z); model = glm::translate(model, glm::vec3(16.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); arbol->Draw(shader);

            model = Base(z - 5.0f); model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); arbol->Draw(shader);
        }
    }

    if (luminaria) {
        for (float z = 0.0f; z > -140.0f; z -= 22.0f) {
            model = Base(z); model = glm::translate(model, glm::vec3(19.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); luminaria->Draw(shader);
        }
    }

    // ==========================================
    // 7. CARRETERA PARALELA A LA PISTA
    //    (los autos y el autobus ahora estan en el cruce, seccion 10)
    // ==========================================
    if (carretera) {
        // Paso de 10 m (pieza de 30 m) para que las piezas se traslapen y sigan bien la curva
        for (float z = 10.0f; z > -170.0f; z -= 10.0f) {
            model = Base(z);
            model = glm::translate(model, glm::vec3(27.0f, 0.01f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            carretera->Draw(shader);
        }
    }

    // ==========================================
    // 8. MOBILIARIO URBANO
    // ==========================================
    if (banca) {
        model = Base(-20.0f); model = glm::translate(model, glm::vec3(2.5f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); banca->Draw(shader);
    }
    if (cono) {
        model = Base(-25.0f); model = glm::translate(model, glm::vec3(5.5f, 0.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); cono->Draw(shader);
    }

    // ==========================================
    // 9. SEMÁFOROS (se alternan: uno en verde, el otro en rojo)
    // ==========================================
    if (semaforo) {
        const float t = static_cast<float>(glfwGetTime());
        const float SEM_X = 21.0f;   // lateral: positivo = hacia la derecha de la ruta

        glm::mat4 m = Base(-45.0f);
        m = glm::translate(m, glm::vec3(SEM_X, 0.0f, 0.0f));
        semaforo->Draw(shader, modelLoc, m, t, 0.0f);

        m = Base(-95.0f);
        m = glm::translate(m, glm::vec3(SEM_X, 0.0f, 0.0f));
        semaforo->Draw(shader, modelLoc, m, t, semaforo->tVerde + semaforo->tAmbar);
        // Sistema local sobre el eje REAL de la ciclovia (sin worldMatrix, que desplaza el resto de Polanco).
// Es el mismo eje con el que CicloviaScene::BuildRoad construye la pista.
        auto BaseRuta = [&](float s) {
            Ruta::Muestra m = Ruta::En(s);
            glm::mat4 b(1.0f);
            b = glm::translate(b, m.pos);
            b = glm::rotate(b, glm::radians(m.headingDeg), glm::vec3(0.0f, 1.0f, 0.0f));
            return b;
            };
    }

    // ==========================================
    // 10. CRUCE DE CARRETERA (animacion activada por la posicion de la bici)
    // ==========================================
    
    {
        const float t = static_cast<float>(glfwGetTime());
        float dt = t - ultimoT;
        ultimoT = t;
        if (dt > 0.1f) dt = 0.1f;
        if (dt < 0.0f) dt = 0.0f;

        // Activacion y reinicio (si la bici vuelve al inicio con R)
        if (cruceT0 < 0.0f && riderS >= CRUCE_S - DIST_ACTIVACION) {
            cruceT0 = t;
        }
        else if (cruceT0 >= 0.0f && riderS < CRUCE_S - DIST_ACTIVACION - 20.0f) {
            cruceT0 = -1.0f;
            for (size_t i = 0; i < vehiculos.size(); i++) vehiculos[i].u = vehiculos[i].u0;
        }
        const bool activo = (cruceT0 >= 0.0f);
        const float tCiclo = activo ? (t - cruceT0) : 0.0f;   // en reposo: bici en verde, autos en rojo

        // Los autos tienen la fase opuesta a la de la bici
        const float desfaseCarros = semaforo ? semaforo->tVerde + semaforo->tAmbar : 0.0f;
        Semaforo::Fase faseCarros = semaforo ? semaforo->FaseEn(tCiclo, desfaseCarros) : Semaforo::ROJO;

        // Si la bici esta sobre el cruce, los carros tratan la luz como roja
        const bool biciEnCruce = std::fabs(riderS - CRUCE_S) < BICI_ZONA;
        if (activo) ActualizarVehiculos(dt, faseCarros == Semaforo::VERDE && !biciEnCruce);

        // Carretera que cruza la ciclovia (120 x 12 m, centrada en el eje de la pista)
        if (carreteraCruce) {
            model = Base(-CRUCE_S);
            model = glm::translate(model, glm::vec3(14.0f, CRUCE_Y, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            carreteraCruce->Draw(shader);
        }

        // Aviso unico en consola: donde esta el cruce en el mundo (compara con la tecla P)
        static bool avisado = false;
        if (!avisado) {
            avisado = true;
            Ruta::Muestra mc = Ruta::En(CRUCE_S);
            std::cout << "[Cruce] centro en (" << mc.pos.x << ", " << mc.pos.y << ", " << mc.pos.z
                << ") | carreteraCruce=" << (carreteraCruce ? "cargada" : "NULL") << std::endl;
        }

       
        // Vehiculos (un carril por sentido)
        for (size_t i = 0; i < vehiculos.size(); i++) {
            const Vehiculo& v = vehiculos[i];
            if (!v.modelo) continue;
            model = Base(-CRUCE_S);

            model = glm::translate(model, glm::vec3(v.u + 8.0f, CRUCE_Y + 0.01f, v.dir * ANCHO_CALLE * 0.25f));

            model = glm::rotate(model, glm::radians(ROT_NARIZ + (v.dir > 0.0f ? 0.0f : 180.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            v.modelo->Draw(shader);
        }
        // Semaforos: uno para la bici y uno por cada sentido de los autos
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