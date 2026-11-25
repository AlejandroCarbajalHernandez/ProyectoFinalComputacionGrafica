#include "TepoztlanScene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <cmath>
#include "Ruta.h"

// Constructor: inicializa punteros de modelos del pueblo en nulo y establece la matriz mundial base
TepoztlanScene::TepoztlanScene() {
    exconvento = nullptr;
    casas = nullptr;
    estacion = nullptr;
    kiosco = nullptr;
    letrero = nullptr;
    mercado = nullptr;
    meta = nullptr;
    cerro = nullptr;
    worldMatrix = glm::mat4(1.0f);
}

// Destructor: libera la memoria dinámica asignada a cada modelo arquitectónico de Tepoztlán
TepoztlanScene::~TepoztlanScene() {
    delete exconvento;
    delete casas;
    delete estacion;
    delete kiosco;
    delete letrero;
    delete mercado;
    delete meta;
    delete cerro;
}

// Carga desde disco los archivos .obj correspondientes a la arquitectura y arco de meta del destino
void TepoztlanScene::Init() {
    exconvento = new Model((char*)"../assets/obj/ExconventoTepoztlan/ExconventoTepoztlan.obj");
    casas = new Model((char*)"../assets/obj/Casas/Casas.obj");
    estacion = new Model((char*)"../assets/obj/EstacionFerrocarril/EstacionFerrocarril.obj");
    kiosco = new Model((char*)"../assets/obj/Kiosco/Kiosco.obj");
    letrero = new Model((char*)"../assets/obj/LetreroTepoztlan/LetreroTepoztlan.obj");
    mercado = new Model((char*)"../assets/obj/MercadoMunicipal/MercadoMunicipal.obj");
    meta = new Model((char*)"../assets/obj/meta/meta.obj");
    cerro = new Model((char*)"../assets/obj/cerro/cerro.obj");
}

// Posiciona y renderiza todos los elementos de Tepoztlán relativos a worldMatrix (metro 475 de la ruta)
void TepoztlanScene::Draw(Shader& shader, GLint modelLoc) {
    glm::mat4 model(1.0f);

    // Función auxiliar que aplica traslación y rotación en Y respecto al origen global del pueblo
    auto Base = [&](glm::vec3 offset, float rotacionY) {
        glm::mat4 m = worldMatrix;
        m = glm::translate(m, offset);
        m = glm::rotate(m, glm::radians(rotacionY), glm::vec3(0.0f, 1.0f, 0.0f));
        return m;
        };

    // 1. Arco de meta en la entrada del pueblo
    if (meta) {
        model = Base(glm::vec3(0.0f, 0.02f, -6.0f), 0.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        meta->Draw(shader);
    }

    // 2. Letrero monumental de bienvenida a Tepoztlán
    if (letrero) {
        model = Base(glm::vec3(13.5f, -0.5f, 5.0f), -18.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        letrero->Draw(shader);
    }

    // 3. Antigua estación del ferrocarril (costado izquierdo)
    if (estacion) {
        model = Base(glm::vec3(-18.0f, 0.3f, -15.0f), 180.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        estacion->Draw(shader);
    }

    // 4. Kiosco central de la plaza
    if (kiosco) {
        model = Base(glm::vec3(10.0f, 0.0f, -25.0f), 0.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        kiosco->Draw(shader);
    }

    // 5. Mercado municipal: se coloca con sus medidas reales y se apoya sobre la altura real del piso en esa zona.
    //    Se ajusta con estas constantes (metros, en el sistema del pueblo: +x derecha, -z hacia adelante).
    if (mercado) {
        const float ROT_MERCADO = -75.0f;
        const float X_BORDE = 26.0f;    // x donde empieza el costado más cercano a la ruta (bájalo para acercarlo)
        const float Z_BORDE = -8.0f;   // z del frente del mercado (más negativo = más adelante)
        const float HUNDIDO = 0.10f;    // cuánto se entierra la base para que no se vea ningún hueco

        static bool listo = false;
        static glm::vec3 off(0.0f);
        if (!listo) {
            glm::vec3 mn, mx;
            mercado->Limites(mn, mx);
            const float c = std::cos(glm::radians(ROT_MERCADO)), s = std::sin(glm::radians(ROT_MERCADO));
            float rx[4], rz[4], minRX = 1e9f, maxRZ = -1e9f;
            for (int i = 0; i < 4; i++) {
                float x = (i & 1) ? mx.x : mn.x;
                float z = (i & 2) ? mx.z : mn.z;
                rx[i] = x * c + z * s;      // x tras girar en Y
                rz[i] = -x * s + z * c;     // z tras girar en Y
                minRX = std::min(minRX, rx[i]);
                maxRZ = std::max(maxRZ, rz[i]);
            }
            off.x = X_BORDE - minRX;
            off.z = Z_BORDE - maxRZ;

            // Altura del piso bajo las esquinas del mercado: se toma la más alta para que no quede flotando
            float sueloMax = -1e9f;
            for (int i = 0; i < 4; i++) {
                glm::vec4 w = worldMatrix * glm::vec4(off.x + rx[i], 0.0f, off.z + rz[i], 1.0f);
                sueloMax = std::max(sueloMax, Ruta::AlturaSuelo(w.x, w.z) - 0.08f);   // el piso se dibuja a AlturaSuelo - 0.08
            }
            // y local = altura del piso - altura del origen del pueblo; la base del modelo (mn.y) queda sobre ese piso
            off.y = (sueloMax - HUNDIDO) - worldMatrix[3].y - mn.y;
            listo = true;
        }
        model = Base(off, ROT_MERCADO);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        mercado->Draw(shader);
    }

    // 6. Exconvento de la Natividad al fondo del recorrido
    if (exconvento) {
        model = Base(glm::vec3(5.5f, 0.4f, -53.0f), 0.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        exconvento->Draw(shader);
    }

    // 7. Conjunto de casas tradicionales del pueblo
    if (casas) {
        model = Base(glm::vec3(-30.0f, 0.3f, -50.0f), 0.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        casas->Draw(shader);
    }
    // 8. Cerro del Tepozteco al fondo 
    if (cerro) {
        model = Base(glm::vec3(-84.0f, -3.0f, -205.0f), 0.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        cerro->Draw(shader);
    }
}
