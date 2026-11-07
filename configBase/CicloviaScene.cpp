#include "CicloviaScene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include "Ruta.h"
#include <algorithm>

CicloviaScene::CicloviaScene() {
    trackModel = nullptr;
    bridgeModel = nullptr;
    treeModel = nullptr;
    mountainModel = nullptr;
    bushModel = nullptr;
    signModel = nullptr;
    trafficConeModel = nullptr;
    building1Model = nullptr;
    building2Model = nullptr;
    terrainVAO = 0;
    terrainVBO = 0;
    terrainCount = 0;
    roadVAO = 0; 
    roadVBO = 0;
}

CicloviaScene::~CicloviaScene() {
    delete trackModel;
    delete bridgeModel;
    delete treeModel;
    delete bushModel;
    delete signModel;
    delete trafficConeModel;
    if (terrainVAO) glDeleteVertexArrays(1, &terrainVAO);
    if (terrainVBO) glDeleteBuffers(1, &terrainVBO);
    if (roadVAO) glDeleteVertexArrays(1, &roadVAO);
    if (roadVBO) glDeleteBuffers(1, &roadVBO);
}

void CicloviaScene::Init() {
    trackModel = new Model((char*)"../assets/obj/pista/pista.obj");
    bridgeModel = new Model((char*)"../assets/obj/puente/puente.obj");
    treeModel = new Model((char*)"../assets/obj/arbol/arbol.obj");
    mountainModel = new Model((char*)"../assets/obj/Monta\xF1" "a/monta\xF1" "a.obj");
    bushModel = new Model((char*)"../assets/obj/arbusto/arbusto.obj");
    trafficConeModel = new Model((char*)"../assets/obj/tope/tope.obj");
    signModel = new Model((char*)"../assets/obj/senal-alto/senal-alto.obj");
    building1Model = new Model((char*)"../assets/obj/edificio-01/edificio-01.obj");
    building2Model = new Model((char*)"../assets/obj/edificio-02/edificio-02.obj");
    

    // Terreno con relieve: sigue la altura de la ruta (cuadricula de 4 m)
    const float X0 = -300.0f, X1 = 150.0f, Z0 = -700.0f, Z1 = 100.0f, PASO_T = 4.0f;
    const int NX = (int)((X1 - X0) / PASO_T), NZ = (int)((Z1 - Z0) / PASO_T);

    std::vector<float> alturas((NX + 1) * (NZ + 1));
    for (int j = 0; j <= NZ; j++)
        for (int i = 0; i <= NX; i++)
            alturas[j * (NX + 1) + i] = Ruta::AlturaSuelo(X0 + i * PASO_T, Z0 + j * PASO_T) - 0.12f;

    auto H = [&](int i, int j) {
        i = std::max(0, std::min(NX, i));
        j = std::max(0, std::min(NZ, j));
        return alturas[j * (NX + 1) + i];
        };

    std::vector<float> ground;
    auto vertice = [&](int i, int j) {
        float dhdx = (H(i + 1, j) - H(i - 1, j)) / (2.0f * PASO_T);
        float dhdz = (H(i, j + 1) - H(i, j - 1)) / (2.0f * PASO_T);
        glm::vec3 n = glm::normalize(glm::vec3(-dhdx, 1.0f, -dhdz));
        float x = X0 + i * PASO_T, z = Z0 + j * PASO_T;
        float f[8] = { x, H(i, j), z, n.x, n.y, n.z, x / 6.0f, z / 6.0f };
        ground.insert(ground.end(), f, f + 8);
        };
    for (int j = 0; j < NZ; j++) {
        for (int i = 0; i < NX; i++) {
            vertice(i, j); vertice(i + 1, j); vertice(i + 1, j + 1);
            vertice(i, j); vertice(i + 1, j + 1); vertice(i, j + 1);
        }
    }
    terrainCount = (int)(ground.size() / 8);

    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glBindVertexArray(terrainVAO);

    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, ground.size() * sizeof(float), ground.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    BuildRoad();
}

void CicloviaScene::Draw(Shader& shader, GLint modelLoc) {
    glm::mat4 model(1.0f);

    // 1. Dibujar el plano de tierra/pasto seco que sostiene la ciclovía
    // (el terreno no tiene textura todavía: se usa un color base verde seco)
    glUniform1i(glGetUniformLocation(shader.Program, "useTexture"), 0);
    glUniform3f(glGetUniformLocation(shader.Program, "baseColor"), 0.35f, 0.42f, 0.18f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(terrainVAO);
    glDrawArrays(GL_TRIANGLES, 0, terrainCount);
    glBindVertexArray(0);

    // 2. Pista que sigue el trazo (asfalto, raya central y bordillos)
    glUniform1i(glGetUniformLocation(shader.Program, "useTexture"), 0);
    model = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(roadVAO);
    glUniform3f(glGetUniformLocation(shader.Program, "baseColor"), 0.18f, 0.18f, 0.20f);
    glDrawArrays(GL_TRIANGLES, roadStart[0], roadCount[0]);
    glUniform3f(glGetUniformLocation(shader.Program, "baseColor"), 0.95f, 0.78f, 0.10f);
    glDrawArrays(GL_TRIANGLES, roadStart[1], roadCount[1]);
    glUniform3f(glGetUniformLocation(shader.Program, "baseColor"), 0.70f, 0.70f, 0.68f);
    glDrawArrays(GL_TRIANGLES, roadStart[2], roadCount[2]);
    glBindVertexArray(0);

    
    // Zonas del recorrido (cada una en su propia funcion)
    DrawUrbana(shader, modelLoc);   // E1 Polanco   (0 a 210 m)
    DrawRural(shader, modelLoc);    // E2 Rural     (210 a 475 m)
    DrawPueblo(shader, modelLoc);   // E3 Tepoztlan (475 a 685 m)
}
void CicloviaScene::BuildRoad() {
        std::vector<float> asfalto, linea, bordillo;

        // Agrega un cuadrilatero (dos triangulos) con normal hacia arriba
        auto quad = [](std::vector<float>& v, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d) {
            glm::vec3 pts[6] = { a, b, c, a, c, d };
            for (int k = 0; k < 6; k++) {
                float f[8] = { pts[k].x, pts[k].y, pts[k].z, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f };
                v.insert(v.end(), f, f + 8);
            }
            };

        // Ancho de la pista: 2.5 m urbana, 2.0 m rural, 2.25 m pueblo (con transiciones suaves)
        auto ancho = [](float s) {
            float w = 2.5f;
            w += (2.0f - 2.5f) * Ruta::SmoothStep(195.0f, 225.0f, s);
            w += (2.25f - 2.0f) * Ruta::SmoothStep(460.0f, 490.0f, s);
            return w;
            };

        const std::vector<Ruta::Muestra>& m = Ruta::Muestras();
        const float yAsfalto = 0.02f, yBordillo = 0.10f, anchoBordillo = 0.18f;

        for (size_t i = 0; i + 1 < m.size(); i++) {
            const Ruta::Muestra& a = m[i];
            const Ruta::Muestra& b = m[i + 1];
            float ta = glm::radians(a.headingDeg), tb = glm::radians(b.headingDeg);
            glm::vec3 ra(std::cos(ta), 0.0f, -std::sin(ta));   // vector hacia la derecha en cada punto
            glm::vec3 rb(std::cos(tb), 0.0f, -std::sin(tb));
            float wa = ancho(a.s) * 0.5f, wb = ancho(b.s) * 0.5f;
            glm::vec3 up(0.0f, 1.0f, 0.0f);

            glm::vec3 La = a.pos - ra * wa, Ra = a.pos + ra * wa;
            glm::vec3 Lb = b.pos - rb * wb, Rb = b.pos + rb * wb;

            // Asfalto
            quad(asfalto, La + up * yAsfalto, Ra + up * yAsfalto, Rb + up * yAsfalto, Lb + up * yAsfalto);

            // Raya central discontinua (1.5 m de raya, 1.5 m de hueco)
            if (std::fmod(a.s, 3.0f) < 1.5f) {
                float h = 0.06f;
                quad(linea, a.pos - ra * h + up * (yAsfalto + 0.01f), a.pos + ra * h + up * (yAsfalto + 0.01f),
                    b.pos + rb * h + up * (yAsfalto + 0.01f), b.pos - rb * h + up * (yAsfalto + 0.01f));
            }

            // Bordillos a los dos lados (cara superior y cara interior)
            for (int lado = -1; lado <= 1; lado += 2) {
                glm::vec3 ea = a.pos + ra * (wa * lado), eb = b.pos + rb * (wb * lado);
                glm::vec3 oa = ea + ra * (anchoBordillo * lado), ob = eb + rb * (anchoBordillo * lado);
                quad(bordillo, ea + up * yBordillo, oa + up * yBordillo, ob + up * yBordillo, eb + up * yBordillo);
                quad(bordillo, ea + up * yAsfalto, ea + up * yBordillo, eb + up * yBordillo, eb + up * yAsfalto);
            }
        }

        // Se guardan las tres partes en un solo buffer
        std::vector<float> todo;
        roadStart[0] = 0;                           roadCount[0] = (int)(asfalto.size() / 8);
        roadStart[1] = roadCount[0];                roadCount[1] = (int)(linea.size() / 8);
        roadStart[2] = roadStart[1] + roadCount[1]; roadCount[2] = (int)(bordillo.size() / 8);
        todo.insert(todo.end(), asfalto.begin(), asfalto.end());
        todo.insert(todo.end(), linea.begin(), linea.end());
        todo.insert(todo.end(), bordillo.begin(), bordillo.end());

        glGenVertexArrays(1, &roadVAO);
        glGenBuffers(1, &roadVBO);
        glBindVertexArray(roadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, roadVBO);
        glBufferData(GL_ARRAY_BUFFER, todo.size() * sizeof(float), todo.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
}


// ============================================================
// E1: Polanco, salida urbana (0 a 210 m de la ruta)
// PENDIENTE (equipo): edificios, museo, semaforos, alumbrado, banquetas, estatuas.
// Para colocar objetos siguiendo la pista usa Ruta::En(s) (punto del eje a s metros).
// ============================================================
void CicloviaScene::DrawUrbana(Shader& shader, GLint modelLoc) {
    glm::mat4 model(1.0f);

    // Puente peatonal
    if (bridgeModel) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -48.0f));
        model = glm::scale(model, glm::vec3(0.22f, 0.22f, 0.22f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        bridgeModel->Draw(shader);
    }

    // Confinadores / topes
    if (trafficConeModel) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.3f, 0.0f, -14.0f));
        model = glm::scale(model, glm::vec3(0.12f, 0.12f, 0.12f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        trafficConeModel->Draw(shader);
    }

    // Señalamiento de cicloturismo
    if (signModel) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.8f, 0.0f, -6.0f));
        model = glm::scale(model, glm::vec3(0.14f, 0.14f, 0.14f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        signModel->Draw(shader);
    }
}

// ============================================================
// E3: Tepoztlan, llegada al pueblo (475 a 685 m de la ruta)
// PENDIENTE (equipo): casas, carpas, mesas, bancas, letreros, mapa, equipaje, perros, gatos.
// ============================================================
void CicloviaScene::DrawPueblo(Shader& shader, GLint modelLoc) {
}


// ============================================================
// E2: Ascenso rural (210 a 475 m de la ruta)
// Arboles y arbustos colocados a lo largo del trazo, siguiendo curvas y subida.
// ============================================================
void CicloviaScene::DrawRural(Shader& shader, GLint modelLoc) {
    glm::mat4 model(1.0f);
    const float INICIO = 210.0f, FIN = 475.0f;   // limites de E2 (metros de recorrido)

    // Punto a 's' metros de recorrido y 'lado' metros a la derecha de la pista (negativo = izquierda)
    auto punto = [](float s, float lado) {
        Ruta::Muestra p = Ruta::En(s);
        float th = glm::radians(p.headingDeg);
        glm::vec3 r = p.pos + glm::vec3(std::cos(th), 0.0f, -std::sin(th)) * lado;
        r.y = Ruta::AlturaSuelo(r.x, r.z) - 0.12f;   // sobre el terreno
        return r;
        };
    // Numero "al azar" entre 0 y 1 que siempre da lo mismo para la misma semilla
    auto azar = [](int k) {
        float x = std::sin(k * 12.9898f) * 43758.5453f;
        return x - std::floor(x);
        };

    // Arboles: uno por lado cada ~9 m, a 7-15 m de la pista, con tamano y giro distintos
    if (treeModel) {
        int k = 0;
        for (float s = INICIO + 6.0f; s < FIN; s += 9.0f, k++) {
            for (int L = 0; L < 2; L++) {
                int semilla = k * 2 + L;
                float sJ = s + azar(semilla) * 6.0f;
                float dist = 7.0f + azar(semilla + 500) * 8.0f;
                float esc = 0.8f + azar(semilla + 900) * 0.5f;
                float giro = azar(semilla + 1300) * 360.0f;
                glm::vec3 p = punto(sJ, (L == 0 ? 1.0f : -1.0f) * dist);

                model = glm::mat4(1.0f);
                model = glm::translate(model, p);
                model = glm::rotate(model, glm::radians(giro), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::scale(model, glm::vec3(esc));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                treeModel->Draw(shader);
            }
        }
    }

    // Arbustos: uno por lado cada ~6 m, a 3.5-6 m de la pista
    if (bushModel) {
        int k = 0;
        for (float s = INICIO + 3.0f; s < FIN; s += 6.0f, k++) {
            for (int L = 0; L < 2; L++) {
                int semilla = 3000 + k * 2 + L;
                float sJ = s + azar(semilla) * 4.0f;
                float dist = 3.5f + azar(semilla + 500) * 2.5f;
                float esc = 0.12f * (0.8f + azar(semilla + 900) * 0.5f);
                glm::vec3 p = punto(sJ, (L == 0 ? 1.0f : -1.0f) * dist);

                model = glm::mat4(1.0f);
                model = glm::translate(model, p);
                model = glm::scale(model, glm::vec3(esc));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                bushModel->Draw(shader);
            }
        }
    }
}