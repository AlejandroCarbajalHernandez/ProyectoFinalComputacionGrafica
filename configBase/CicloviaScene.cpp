#include "CicloviaScene.h"
#include <GLFW/glfw3.h>
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

    huizacheModel = nullptr;
    nopalModel = nullptr;
    magueyModel = nullptr;
    zacatonModel = nullptr;
    gavillaModel = nullptr;
    tepetateModel = nullptr;
    torreModel = nullptr;

    perroModel = nullptr;
    cabraModel = nullptr;
    ovejaModel = nullptr;
    caballoModel = nullptr;

    terrainVAO = 0;
    terrainVBO = 0;
    terrainCount = 0;
    aridVAO = 0;
    aridVBO = 0;
    aridCount = 0;
    roadVAO = 0;
    roadVBO = 0;

    for (int i = 0; i < 4; i++) {
        roadStart[i] = 0;
        roadCount[i] = 0;
    }
}

CicloviaScene::~CicloviaScene() {
    delete trackModel;
    delete bridgeModel;
    delete treeModel;
    delete bushModel;
    delete signModel;
    delete trafficConeModel;
    delete building1Model;
    delete building2Model;

    delete huizacheModel;
    delete nopalModel;
    delete magueyModel;
    delete zacatonModel;
    delete gavillaModel;
    delete tepetateModel;
    delete torreModel;

    delete perroModel;
    delete cabraModel;
    delete ovejaModel;
    delete caballoModel;

    if (terrainVAO) glDeleteVertexArrays(1, &terrainVAO);
    if (terrainVBO) glDeleteBuffers(1, &terrainVBO);
    if (aridVAO)    glDeleteVertexArrays(1, &aridVAO);
    if (aridVBO)    glDeleteBuffers(1, &aridVBO);
    if (roadVAO)    glDeleteVertexArrays(1, &roadVAO);
    if (roadVBO)    glDeleteBuffers(1, &roadVBO);
}

void CicloviaScene::Init() {
    trackModel = new Model((char*)"../assets/obj/pista/pista.obj");
    treeModel = new Model((char*)"../assets/obj/arbol/arbol.obj"); // Ocote
    mountainModel = new Model((char*)"../assets/obj/Monta\xF1" "a/monta\xF1" "a.obj");
    bushModel = new Model((char*)"../assets/obj/arbusto/arbusto.obj");
    building1Model = new Model((char*)"../assets/obj/edificio-01/edificio-01.obj");
    building2Model = new Model((char*)"../assets/obj/edificio-02/edificio-02.obj");

    // Elementos del bioma árido / Morelos
    huizacheModel = new Model((char*)"../assets/obj/Huizache/Huizache.obj");
    nopalModel = new Model((char*)"../assets/obj/Nopal Silvestre/NopalSilvestre.obj");
    magueyModel = new Model((char*)"../assets/obj/Maguey/maguey.obj");
    zacatonModel = new Model((char*)"../assets/obj/zacaton/zacaton.obj");
    gavillaModel = new Model((char*)"../assets/obj/Gavilla/Gavilla.obj");
    tepetateModel = new Model((char*)"../assets/obj/Tepetate/Tepetate.obj");
    torreModel = new Model((char*)"../assets/obj/torre-alta-tension/torre-alta-tension.obj");

    // Fauna
    perroModel = new Model((char*)"../assets/obj/perro/perro.obj");
    cabraModel = new Model((char*)"../assets/obj/Cabra/Cabra.obj");
    ovejaModel = new Model((char*)"../assets/obj/oveja/Oveja.obj");
    caballoModel = new Model((char*)"../assets/obj/caballo/caballo.obj");

    // Terreno base continuo verde
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
    BuildAridTerrain();

    Ruta::Muestra p0 = Ruta::En(0.0f);
    const PolancoScene::Viewpoint& vp = PolancoScene::GetViewpoint(0);
    glm::mat4 w(1.0f);
    w = glm::translate(w, p0.pos);
    w = glm::rotate(w, glm::radians(p0.headingDeg - vp.rotDeg), glm::vec3(0.0f, 1.0f, 0.0f));
    w = glm::translate(w, -vp.pos);
    polanco.worldMatrix = w;
    polanco.Init();
}

// Manto árido estrictamente entre 320 y 415 m
void CicloviaScene::BuildAridTerrain() {
    std::vector<float> aridVerts;
    const float S_INICIO_ARIDO = 320.0f;
    const float S_FIN_ARIDO = 415.0f;
    const float PASO_S = 2.5f;
    const float ANCHO_LATERAL = 26.0f;
    const int   SUBDIV_LAT = 6;

    auto getPuntoArido = [](float s, float lado) {
        Ruta::Muestra p = Ruta::En(s);
        float th = glm::radians(p.headingDeg);
        glm::vec3 pos = p.pos + glm::vec3(std::cos(th), 0.0f, -std::sin(th)) * lado;
        pos.y = Ruta::AlturaSuelo(pos.x, pos.z) - 0.08f;
        return pos;
        };

    for (float s = S_INICIO_ARIDO; s + PASO_S <= S_FIN_ARIDO; s += PASO_S) {
        float s0 = s;
        float s1 = s + PASO_S;

        for (int d = 0; d < SUBDIV_LAT; d++) {
            float l0 = -ANCHO_LATERAL + (2.0f * ANCHO_LATERAL / SUBDIV_LAT) * d;
            float l1 = -ANCHO_LATERAL + (2.0f * ANCHO_LATERAL / SUBDIV_LAT) * (d + 1);

            glm::vec3 v00 = getPuntoArido(s0, l0);
            glm::vec3 v10 = getPuntoArido(s1, l0);
            glm::vec3 v01 = getPuntoArido(s0, l1);
            glm::vec3 v11 = getPuntoArido(s1, l1);

            glm::vec3 n = glm::normalize(glm::cross(v10 - v00, v01 - v00));
            if (n.y < 0.0f) n = -n;

            auto pushV = [&](const glm::vec3& p) {
                float f[8] = { p.x, p.y, p.z, n.x, n.y, n.z, p.x / 5.0f, p.z / 5.0f };
                aridVerts.insert(aridVerts.end(), f, f + 8);
                };

            pushV(v00); pushV(v10); pushV(v11);
            pushV(v00); pushV(v11); pushV(v01);
        }
    }

    aridCount = (int)(aridVerts.size() / 8);

    glGenVertexArrays(1, &aridVAO);
    glGenBuffers(1, &aridVBO);
    glBindVertexArray(aridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, aridVBO);
    glBufferData(GL_ARRAY_BUFFER, aridVerts.size() * sizeof(float), aridVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

// Construye Asfalto, Raya, Bordillos y Terracería (415 m a 475 m)
void CicloviaScene::BuildRoad() {
    std::vector<float> asfalto, linea, bordillo, terraceria;

    auto quad = [](std::vector<float>& v, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d) {
        glm::vec3 pts[6] = { a, b, c, a, c, d };
        for (int k = 0; k < 6; k++) {
            float f[8] = { pts[k].x, pts[k].y, pts[k].z, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f };
            v.insert(v.end(), f, f + 8);
        }
        };

    auto ancho = [](float s) {
        float w = 2.5f;
        w += (2.0f - 2.5f) * Ruta::SmoothStep(195.0f, 225.0f, s);
        w += (2.25f - 2.0f) * Ruta::SmoothStep(460.0f, 490.0f, s);
        return w;
        };

    const std::vector<Ruta::Muestra>& m = Ruta::Muestras();
    const float yPista = 0.02f, yBordillo = 0.10f, anchoBordillo = 0.18f;
    const float S_CORTE_TERRACERIA = 415.0f; // Asfalto termina aquí; inicia terracería rústica

    for (size_t i = 0; i + 1 < m.size(); i++) {
        const Ruta::Muestra& a = m[i];
        const Ruta::Muestra& b = m[i + 1];
        float ta = glm::radians(a.headingDeg), tb = glm::radians(b.headingDeg);
        glm::vec3 ra(std::cos(ta), 0.0f, -std::sin(ta));
        glm::vec3 rb(std::cos(tb), 0.0f, -std::sin(tb));
        float wa = ancho(a.s) * 0.5f, wb = ancho(b.s) * 0.5f;
        glm::vec3 up(0.0f, 1.0f, 0.0f);

        glm::vec3 La = a.pos - ra * wa, Ra = a.pos + ra * wa;
        glm::vec3 Lb = b.pos - rb * wb, Rb = b.pos + rb * wb;

        if (a.s < S_CORTE_TERRACERIA) {
            // TRAMO 1: ASFALTO URBANO Y RURAL
            quad(asfalto, La + up * yPista, Ra + up * yPista, Rb + up * yPista, Lb + up * yPista);

            // Raya amarilla central
            if (std::fmod(a.s, 3.0f) < 1.5f) {
                float h = 0.06f;
                quad(linea, a.pos - ra * h + up * (yPista + 0.01f), a.pos + ra * h + up * (yPista + 0.01f),
                    b.pos + rb * h + up * (yPista + 0.01f), b.pos - rb * h + up * (yPista + 0.01f));
            }

            // Bordillos de concreto
            for (int lado = -1; lado <= 1; lado += 2) {
                glm::vec3 ea = a.pos + ra * (wa * lado), eb = b.pos + rb * (wb * lado);
                glm::vec3 oa = ea + ra * (anchoBordillo * lado), ob = eb + rb * (anchoBordillo * lado);
                quad(bordillo, ea + up * yBordillo, oa + up * yBordillo, ob + up * yBordillo, eb + up * yBordillo);
                quad(bordillo, ea + up * yPista, ea + up * yBordillo, eb + up * yBordillo, eb + up * yPista);
            }
        }
        else {
            // TRAMO 2: TERRACERÍA PREVIA A TEPOZTLÁN (sin raya y sin bordillos de concreto)
            quad(terraceria, La + up * yPista, Ra + up * yPista, Rb + up * yPista, Lb + up * yPista);
        }
    }

    std::vector<float> todo;
    roadStart[0] = 0;                                           roadCount[0] = (int)(asfalto.size() / 8);
    roadStart[1] = roadCount[0];                                roadCount[1] = (int)(linea.size() / 8);
    roadStart[2] = roadStart[1] + roadCount[1];                 roadCount[2] = (int)(bordillo.size() / 8);
    roadStart[3] = roadStart[2] + roadCount[2];                 roadCount[3] = (int)(terraceria.size() / 8);

    todo.insert(todo.end(), asfalto.begin(), asfalto.end());
    todo.insert(todo.end(), linea.begin(), linea.end());
    todo.insert(todo.end(), bordillo.begin(), bordillo.end());
    todo.insert(todo.end(), terraceria.begin(), terraceria.end());

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

void CicloviaScene::Draw(Shader& shader, GLint modelLoc) {
    glm::mat4 model(1.0f);

    // 1. Suelo base: Verde monte
    glUniform1i(glGetUniformLocation(shader.Program, "useColor"), 1);
    glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), 0.35f, 0.44f, 0.20f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(terrainVAO);
    glDrawArrays(GL_TRIANGLES, 0, terrainCount);
    glBindVertexArray(0);

    // 2. Manto árido: Color tepetate / tierra arcillosa (320 a 415 m)
    glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), 0.62f, 0.50f, 0.33f);
    glBindVertexArray(aridVAO);
    glDrawArrays(GL_TRIANGLES, 0, aridCount);
    glBindVertexArray(0);

    // 3. Pista segmentada
    model = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(roadVAO);

    // Asfalto
    glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), 0.30f, 0.30f, 0.32f);
    glDrawArrays(GL_TRIANGLES, roadStart[0], roadCount[0]);

    // Raya amarilla
    glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), 0.95f, 0.78f, 0.10f);
    glDrawArrays(GL_TRIANGLES, roadStart[1], roadCount[1]);

    // Bordillos de concreto
    glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), 0.70f, 0.70f, 0.68f);
    glDrawArrays(GL_TRIANGLES, roadStart[2], roadCount[2]);

    // Terracería apisonada (415 a 475 m)
    glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), 0.48f, 0.38f, 0.26f);
    glDrawArrays(GL_TRIANGLES, roadStart[3], roadCount[3]);

    glBindVertexArray(0);

    glUniform1i(glGetUniformLocation(shader.Program, "useColor"), 0);

    DrawUrbana(shader, modelLoc);
    DrawRural(shader, modelLoc);
    DrawPueblo(shader, modelLoc);
}

void CicloviaScene::DrawUrbana(Shader& shader, GLint modelLoc) {
    polanco.Draw(shader, modelLoc);
}

void CicloviaScene::DrawPueblo(Shader& shader, GLint modelLoc) {
    // E3: Inicia a los 475 m en la cota de 26 m de altura
}

// ==============================================================================
// E2: Recorrido Rural (210 a 475 m)
// - Distribución estratificada por franjas (CERO solapamientos)
// - Animales caminando y pastando en trayectorias elípticas seguras (> 4.0 m)
// ==============================================================================
void CicloviaScene::DrawRural(Shader& shader, GLint modelLoc) {
    glm::mat4 model(1.0f);
    const float INICIO_E2 = 210.0f;
    const float FIN_BOSQUE = 320.0f;
    const float INICIO_ARIDO = 320.0f;
    const float FIN_ARIDO = 415.0f; // Fin exacto del bioma árido
    const float FIN_E2 = 475.0f; // Cota de 26 m

    auto punto = [](float s, float lado) {
        Ruta::Muestra p = Ruta::En(s);
        float th = glm::radians(p.headingDeg);
        glm::vec3 r = p.pos + glm::vec3(std::cos(th), 0.0f, -std::sin(th)) * lado;
        r.y = Ruta::AlturaSuelo(r.x, r.z) - 0.12f;
        return r;
        };

    auto azar = [](int k) {
        float x = std::sin(k * 12.9898f) * 43758.5453f;
        return x - std::floor(x);
        };

    // -------------------------------------------------------------------------
    // 1. BOSQUE DE OCOTES Y ARBUSTOS (210 a 320 m)
    // -------------------------------------------------------------------------
    if (treeModel) {
        int k = 0;
        for (float s = INICIO_E2 + 5.0f; s < FIN_BOSQUE; s += 8.0f, k++) {
            for (int L = 0; L < 2; L++) {
                int sem = k * 2 + L;
                float sJ = s + azar(sem) * 3.0f;
                float dist = 7.5f + azar(sem + 500) * 7.5f; // Bien alejado de la pista
                float esc = 0.90f + azar(sem + 900) * 0.25f;
                float giro = azar(sem + 1300) * 360.0f;
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

    if (bushModel) {
        int k = 0;
        for (float s = INICIO_E2 + 3.0f; s < FIN_BOSQUE; s += 6.0f, k++) {
            for (int L = 0; L < 2; L++) {
                int sem = 3000 + k * 2 + L;
                float sJ = s + azar(sem) * 2.5f;
                float dist = 4.2f + azar(sem + 500) * 2.5f; // > 4.0 m
                float esc = 0.12f * (0.90f + azar(sem + 900) * 0.20f);
                glm::vec3 p = punto(sJ, (L == 0 ? 1.0f : -1.0f) * dist);

                model = glm::mat4(1.0f);
                model = glm::translate(model, p);
                model = glm::scale(model, glm::vec3(esc));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                bushModel->Draw(shader);
            }
        }
    }

    // -------------------------------------------------------------------------
    // 2. BIOMA ÁRIDO (320 a 415 m) SIN SOLAPAMIENTOS
    // Franja Media: Se alterna Huizache (slot par) o Maguey (slot impar)
    // Franja Corta: Nopal o Tepetate con Zacatón de relleno
    // -------------------------------------------------------------------------
    int slot = 0;
    for (float s = INICIO_ARIDO + 3.0f; s < FIN_ARIDO; s += 5.5f, slot++) {
        for (int L = 0; L < 2; L++) {
            int sem = slot * 2 + L;
            float signo = (L == 0 ? 1.0f : -1.0f);

            // FRANJA MEDIA (7.0 m a 12.0 m): Alternancia Huizache vs Maguey
            if (slot % 2 == 0) {
                if (huizacheModel) {
                    float dist = 7.5f + azar(sem + 100) * 4.5f;
                    glm::vec3 p = punto(s + azar(sem) * 2.0f, signo * dist);
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, p);
                    model = glm::rotate(model, glm::radians(azar(sem + 200) * 360.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    model = glm::scale(model, glm::vec3(0.95f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                    huizacheModel->Draw(shader);
                }
            }
            else {
                if (magueyModel) {
                    float dist = 7.0f + azar(sem + 150) * 4.0f;
                    glm::vec3 p = punto(s + azar(sem) * 2.0f, signo * dist);
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, p);
                    model = glm::rotate(model, glm::radians(azar(sem + 250) * 360.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    model = glm::scale(model, glm::vec3(0.90f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                    magueyModel->Draw(shader);
                }
            }

            // FRANJA CORTA (4.2 m a 6.2 m): Nopal vs Roca de Tepetate
            if (slot % 2 == 0) {
                if (nopalModel) {
                    float dist = 4.8f + azar(sem + 300) * 1.6f; // Siempre > 4.5 m
                    glm::vec3 p = punto(s + 2.5f, signo * dist);
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, p);
                    model = glm::rotate(model, glm::radians(azar(sem + 350) * 360.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    model = glm::scale(model, glm::vec3(0.90f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                    nopalModel->Draw(shader);
                }
            }
            else {
                if (tepetateModel) {
                    float dist = 4.3f + azar(sem + 400) * 1.5f;
                    glm::vec3 p = punto(s + 2.5f, signo * dist);
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, p);
                    model = glm::rotate(model, glm::radians(azar(sem + 450) * 360.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    model = glm::scale(model, glm::vec3(0.85f));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                    tepetateModel->Draw(shader);
                }
            }

            // Zacatón seco de relleno orgánico (desplazado en s)
            if (zacatonModel) {
                float distZ = 4.0f + azar(sem + 600) * 3.5f;
                glm::vec3 pZ = punto(s + 1.2f, signo * distZ);
                model = glm::mat4(1.0f);
                model = glm::translate(model, pZ);
                model = glm::rotate(model, glm::radians(azar(sem + 650) * 360.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::scale(model, glm::vec3(0.85f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                zacatonModel->Draw(shader);
            }
        }
    }

    // Gavillas agrícolas en parcelas alejadas (> 14.0 m)
    if (gavillaModel) {
        for (float sG = 335.0f; sG <= 405.0f; sG += 22.0f) {
            for (int L = 0; L < 2; L++) {
                glm::vec3 pGav = punto(sG, (L == 0 ? 1.0f : -1.0f) * 15.0f);
                model = glm::mat4(1.0f);
                model = glm::translate(model, pGav);
                model = glm::scale(model, glm::vec3(1.15f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                gavillaModel->Draw(shader);
            }
        }
    }

    // Torres de alta tensión (a 38 m de la vía)
    if (torreModel) {
        for (float sT = 325.0f; sT <= 405.0f; sT += 45.0f) {
            glm::vec3 pTorre = punto(sT, 38.0f);
            Ruta::Muestra mTorre = Ruta::En(sT);
            model = glm::mat4(1.0f);
            model = glm::translate(model, pTorre);
            model = glm::rotate(model, glm::radians(mTorre.headingDeg), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            torreModel->Draw(shader);
        }
    }

    // -------------------------------------------------------------------------
    // 3. FAUNA EN MOVIMIENTO (CAMINATA Y PASTOREO PROCEDURAL)
    // Trayectorias elípticas locales con cabeceo de paso. Siempre a > 4.2 m del eje.
    // -------------------------------------------------------------------------
    float t = (float)glfwGetTime();
    const float S_REBANO = 370.0f;
    Ruta::Muestra pMuestra = Ruta::En(S_REBANO);
    float rumboBase = pMuestra.headingDeg;

    // A. Ovejas: Pequeño recorrido elíptico pastando con cadencia de pasos
    if (ovejaModel) {
        const struct { float ds; float lateral; float rot; float fOff; float vel; } ovData[] = {
            { -4.0f,  4.5f,  30.0f, 0.0f, 0.45f }, { -2.5f,  5.2f,  85.0f, 1.2f, 0.50f },
            { -1.0f,  4.4f, -20.0f, 2.5f, 0.40f }, {  0.5f,  5.5f,  45.0f, 3.8f, 0.55f },
            {  2.0f,  4.8f, 110.0f, 0.7f, 0.48f }, {  3.8f,  5.3f,  60.0f, 2.1f, 0.42f },
            { -3.0f, -4.8f, -40.0f, 1.9f, 0.46f }, { -0.5f, -5.5f, -75.0f, 3.1f, 0.52f },
            {  1.5f, -4.7f, -10.0f, 0.4f, 0.44f }, {  3.2f, -5.2f, -60.0f, 2.7f, 0.47f }
        };

        for (const auto& ov : ovData) {
            // Desplazamiento de caminata suave (radio 0.8 m en s, 0.4 m lateral)
            float caminataS = std::sin(t * ov.vel + ov.fOff) * 0.8f;
            float caminataL = std::cos(t * ov.vel + ov.fOff) * 0.4f;
            // Elevación vertical con cada paso al caminar
            float pasoY = std::abs(std::sin(t * ov.vel * 4.0f + ov.fOff)) * 0.025f;

            glm::vec3 p = punto(S_REBANO + ov.ds + caminataS, ov.lateral + caminataL);
            p.y += pasoY;

            // Orientación según la dirección de caminata
            float giroMarcha = std::cos(t * ov.vel + ov.fOff) * 20.0f;
            float respiro = 1.0f + std::sin(t * 2.2f + ov.fOff) * 0.015f;

            model = glm::mat4(1.0f);
            model = glm::translate(model, p);
            model = glm::rotate(model, glm::radians(rumboBase + ov.rot + giroMarcha), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, respiro));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            ovejaModel->Draw(shader);
        }
    }

    // B. Cabras: Trepando y cambiando de posición en el talud
    if (cabraModel) {
        const struct { float ds; float lateral; float rot; float fOff; } cbData[] = {
            { -3.5f,  6.2f, -15.0f, 0.5f }, {  0.0f,  6.6f,  40.0f, 2.3f },
            {  2.8f,  6.0f, -45.0f, 1.1f }, { -1.8f, -6.2f,  20.0f, 3.4f }
        };
        for (const auto& cb : cbData) {
            float camS = std::sin(t * 0.6f + cb.fOff) * 1.1f;
            float pasoY = std::abs(std::sin(t * 2.4f + cb.fOff)) * 0.035f;

            glm::vec3 p = punto(S_REBANO + cb.ds + camS, cb.lateral);
            p.y += pasoY;
            float giroAlerta = std::sin(t * 0.6f + cb.fOff) * 25.0f;

            model = glm::mat4(1.0f);
            model = glm::translate(model, p);
            model = glm::rotate(model, glm::radians(rumboBase + cb.rot + giroAlerta), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            cabraModel->Draw(shader);
        }
    }

    // C. Perros pastores: Trote de patrullaje vigilando el flanco
    if (perroModel) {
        // Perro 1: Trote corto de ida y vuelta a la derecha
        float patrulla1 = std::sin(t * 0.8f) * 1.4f;
        float troteP1 = std::abs(std::sin(t * 4.8f)) * 0.04f;
        float rumboPerro1 = (std::cos(t * 0.8f) >= 0.0f) ? 0.0f : 180.0f;

        glm::vec3 pP1 = punto(S_REBANO - 5.5f + patrulla1, 4.4f);
        pP1.y += troteP1;

        model = glm::mat4(1.0f);
        model = glm::translate(model, pP1);
        model = glm::rotate(model, glm::radians(rumboBase - 20.0f + rumboPerro1), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        perroModel->Draw(shader);

        // Perro 2: Sentado / echado vigilante en el flanco izquierdo
        glm::vec3 pP2 = punto(S_REBANO + 2.5f, -4.6f);
        float respiroP2 = 1.0f + std::sin(t * 3.5f) * 0.02f;
        model = glm::mat4(1.0f);
        model = glm::translate(model, pP2);
        model = glm::rotate(model, glm::radians(rumboBase + 45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, respiroP2, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        perroModel->Draw(shader);
    }

    // D. Caballo castaño: Paso lento y majestuoso en el campo
    if (caballoModel) {
        float pasoCaballoS = std::sin(t * 0.35f) * 1.5f;
        float pasoCaballoL = std::cos(t * 0.35f) * 0.5f;
        float elevacionPaso = std::abs(std::sin(t * 1.4f)) * 0.045f;
        float giroPaso = std::cos(t * 0.35f) * 15.0f;

        glm::vec3 pCaballo = punto(S_REBANO + 6.5f + pasoCaballoS, 5.4f + pasoCaballoL);
        pCaballo.y += elevacionPaso;

        model = glm::mat4(1.0f);
        model = glm::translate(model, pCaballo);
        model = glm::rotate(model, glm::radians(rumboBase - 20.0f + giroPaso), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        caballoModel->Draw(shader);
    }
}