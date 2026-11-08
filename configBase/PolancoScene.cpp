#include "PolancoScene.h"
#include "Ruta.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
}

PolancoScene::~PolancoScene() {
    delete museo; delete banca; delete cono; delete puente; delete vias;
    delete arbol; delete luminaria; delete camellon;
    delete autoRojo; delete autoNegro; delete autobus;
    delete edificioA; delete edificioB; delete edificioBeige; delete edificioC;
    delete techoVerde; delete torreCil; delete torreBal; delete torreCrisA; delete torreCrisB;
    delete carretera;
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
    // 4. EDIFICIOS LADO IZQUIERDO (Franja lateral izquierda, sin encimarse)
    // ==========================================
    if (edificioA) {
        for (float z = 20.0f; z >= -160.0f; z -= 135.0f) { // z = 20 y -115 (antes y después del museo)
            model = Base(z);
            model = glm::translate(model, glm::vec3(-20.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            edificioA->Draw(shader);
        }
    }

    if (edificioB) {
        for (float z = -10.0f; z >= -150.0f; z -= 130.0f) { // z = -10 y -140 (detrás del museo)
            model = Base(z);
            model = glm::translate(model, glm::vec3(-60.0f, 0.0f, 0.0f)); // Segunda línea exterior izquierda
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            edificioB->Draw(shader);
        }
    }

    if (edificioC) {
        model = Base(-150.0f); model = glm::translate(model, glm::vec3(-20.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); edificioC->Draw(shader);
    }

    if (techoVerde) {
        model = Base(-30.0f); model = glm::translate(model, glm::vec3(-55.0f, 0.0f, 0.0f)); // Al frente del museo, lejos de la pista
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); techoVerde->Draw(shader);
    }

    if (torreCrisB) {
        for (float z = -30.0f; z >= -160.0f; z -= 50.0f) { // z = -30, -80, -130
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
    

   

    // 5. EDIFICIOS DERECHOS: 40 -> 43  y  60 -> 63
    // torreBal, edificioBeige -> 43   |   torreCil, torreCrisA -> 63
    if (luminaria) {
        for (float z = 0.0f; z > -140.0f; z -= 22.0f) {
            model = Base(z); model = glm::translate(model, glm::vec3(19.0f, 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); luminaria->Draw(shader);
        }
    }

    // ==========================================
    // 7. CARRETERA Y TRÁFICO (paralela a la pista, entre árboles y edificios derechos)
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

    if (autoRojo) {
        model = Base(-15.0f); model = glm::translate(model, glm::vec3(8.5f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); autoRojo->Draw(shader);
    }
    if (autoNegro) {
        model = Base(-60.0f); model = glm::translate(model, glm::vec3(8.5f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); autoNegro->Draw(shader);

        model = Base(-110.0f); model = glm::translate(model, glm::vec3(8.5f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); autoNegro->Draw(shader);
    }
    if (autobus) {
        model = Base(-85.0f); model = glm::translate(model, glm::vec3(8.5f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); autobus->Draw(shader);
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
}