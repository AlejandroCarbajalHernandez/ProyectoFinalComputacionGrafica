#include "CicloviaScene.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include "Ruta.h"
#include "AvesPose.h"
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
    aveModel = nullptr;
    semaforoModel = nullptr;

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
    delete aveModel;
    delete semaforoModel;

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
    aveModel = new Model((char*)"../assets/obj/bird.obj");
    aveModel->AssignBones(AvesPose::BoneForVertex);
    semaforoModel = new Model((char*)"../assets/obj/semaforo/semaforo.obj");

    // Fauna rural
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

// Manto árido estrictamente entre 320 y 415 m (deja verde el resto hacia Tepoztlán)
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

// Construye Asfalto (0-415 m) y Terracería rústica (415-475 m)
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
    const float S_CORTE_TERRACERIA = 415.0f;

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
            // Asfalto
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
            // Terracería rústica (sin bordillo ni raya)
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

    // 2. Manto árido: Color tepetate (320 a 415 m)
    glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), 0.62f, 0.50f, 0.33f);
    glBindVertexArray(aridVAO);
    glDrawArrays(GL_TRIANGLES, 0, aridCount);
    glBindVertexArray(0);

    // 3. Pista segmentada
    model = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(roadVAO);

    glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), 0.30f, 0.30f, 0.32f);
    glDrawArrays(GL_TRIANGLES, roadStart[0], roadCount[0]);

    glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), 0.95f, 0.78f, 0.10f);
    glDrawArrays(GL_TRIANGLES, roadStart[1], roadCount[1]);

    glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), 0.70f, 0.70f, 0.68f);
    glDrawArrays(GL_TRIANGLES, roadStart[2], roadCount[2]);

    glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), 0.48f, 0.38f, 0.26f);
    glDrawArrays(GL_TRIANGLES, roadStart[3], roadCount[3]);

    glBindVertexArray(0);

    // Restaurar texturas / colores MTL de los modelos OBJ
    glUniform1i(glGetUniformLocation(shader.Program, "useColor"), 0);

    DrawUrbana(shader, modelLoc);
    DrawRural(shader, modelLoc);
    DrawPueblo(shader, modelLoc);
    DrawAves(shader, modelLoc);
}
void CicloviaScene::DrawAves(Shader& shader, GLint modelLoc) {
    float t = (float)glfwGetTime();

    // Centro del vuelo: sobre el eje de la ruta en plena zona rural
    glm::vec3 c = Ruta::En(340.0f).pos;
    c.y = Ruta::AlturaSuelo(c.x, c.z) + 12.0f;

    glUniform1i(glGetUniformLocation(shader.Program, "useBones"), 1);

    for (int i = 0; i < 3; i++) {
        float fase = i * 2.094f;                    // 120 grados entre aves
        float a = t * 0.35f + fase;                 // angulo en la circunferencia
        float radio = 18.0f + i * 5.0f;
        glm::vec3 p = c + glm::vec3(radio * cosf(a), 1.5f * sinf(t * 0.9f + fase), radio * sinf(a));

        // El ave mira hacia -z; theta = pi - a la orienta con la tangente del circulo
        float theta = glm::pi<float>() - a;
        glm::mat4 m(1.0f);
        m = glm::translate(m, p);
        m = glm::rotate(m, theta, glm::vec3(0, 1, 0));
        m = glm::rotate(m, glm::radians(-12.0f), glm::vec3(0, 0, 1));   // inclinacion en la curva
        m = glm::scale(m, glm::vec3(0.12f));
        m = glm::translate(m, glm::vec3(-AvesPose::CX, -AvesPose::BISAGRA_Y, -AvesPose::BISAGRA_Z));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));

        glm::mat4 bones[AvesPose::BONE_COUNT];
        AvesPose::Compute(t + fase, bones);          // cada ave con su propio desfase de aleteo
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "bones"), AvesPose::BONE_COUNT, GL_FALSE, glm::value_ptr(bones[0]));

        aveModel->Draw(shader);
    }
    glUniform1i(glGetUniformLocation(shader.Program, "useBones"), 0);
}


void CicloviaScene::DrawSemaforos(Shader& shader, GLint modelLoc) {
    if (!semaforoModel) return;

    // Ciclo: verde 6 s, ambar 2 s, rojo 6 s
    const float T_VERDE = 6.0f, T_AMBAR = 2.0f, T_ROJO = 6.0f;
    float ciclo = std::fmod((float)glfwGetTime(), T_VERDE + T_AMBAR + T_ROJO);
    bool verde = ciclo < T_VERDE;
    bool ambar = !verde && ciclo < T_VERDE + T_AMBAR;
    bool rojo = !verde && !ambar;

    // Encendida = color brillante, apagada = casi negra
    auto luz = [](bool on, glm::vec3 c) { return on ? c * 1.5f : c * 0.12f; };
    semaforoModel->SetColorMatching("Lente_Rojo", luz(rojo, glm::vec3(0.9f, 0.1f, 0.1f)));
    semaforoModel->SetColorMatching("Lente_Ambar", luz(ambar, glm::vec3(0.9f, 0.6f, 0.0f)));
    semaforoModel->SetColorMatching("Lente_Verde", luz(verde, glm::vec3(0.1f, 0.9f, 0.2f)));

    const float posiciones[2] = { 55.0f, 190.0f };   // metros sobre la ruta (zona urbana)
    for (int i = 0; i < 2; i++) {
        Ruta::Muestra p = Ruta::En(posiciones[i]);
        float th = glm::radians(p.headingDeg);
        glm::vec3 pos = p.pos + glm::vec3(std::cos(th), 0.0f, -std::sin(th)) * 3.2f;   // a la derecha de la pista
        pos.y = Ruta::AlturaSuelo(pos.x, pos.z);

        glm::mat4 m(1.0f);
        m = glm::translate(m, pos);
        m = glm::rotate(m, glm::radians(p.headingDeg + 180.0f), glm::vec3(0, 1, 0));   // las luces miran al ciclista
        m = glm::scale(m, glm::vec3(1.4f));
        m = glm::translate(m, glm::vec3(2.2f, 0.0f, 0.0f));                           // el poste del modelo esta en x = -2.2
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
        semaforoModel->Draw(shader);
    }
}

void CicloviaScene::DrawUrbana(Shader& shader, GLint modelLoc) {
    polanco.Draw(shader, modelLoc);
    DrawSemaforos(shader, modelLoc);
}

void CicloviaScene::DrawPueblo(Shader& shader, GLint modelLoc) {
}

void CicloviaScene::DrawCuadrupedo(Model* m, Shader& shader, GLint modelLoc,
    const glm::mat4& root, float walkPhase,
    float xHombro, float xCadera, float yCadera,
    float ampDeg)
{
    if (!m) return;

    // 1. Dibuja el cuerpo y cabeza excluyendo las patas con 3 argumentos exactos:
    // (Shader, primer filtro, segundo filtro)
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(root));
    m->DrawExcluding(shader, "Pata_D", "Pata_T");

    // 2. Ángulos de oscilación diagonal cruzada
    float angDI = std::sin(walkPhase) * ampDeg;
    float angTD = std::sin(walkPhase) * (ampDeg * 0.90f);
    float angDD = -std::sin(walkPhase) * ampDeg;
    float angTI = -std::sin(walkPhase) * (ampDeg * 0.90f);

    struct ArticulacionPata {
        const char* nombre;
        float angulo;
        float xPivot;
    };

    ArticulacionPata patas[4] = {
        { "Pata_DI", angDI, xHombro },
        { "Pata_DD", angDD, xHombro },
        { "Pata_TI", angTI, xCadera },
        { "Pata_TD", angTD, xCadera }
    };

    // 3. Renderizado de cada extremidad
    for (int p = 0; p < 4; p++) {
        glm::mat4 mPata = root;
        mPata = glm::translate(mPata, glm::vec3(patas[p].xPivot, yCadera, 0.0f));
        mPata = glm::rotate(mPata, glm::radians(patas[p].angulo), glm::vec3(0.0f, 0.0f, 1.0f));
        mPata = glm::translate(mPata, glm::vec3(-patas[p].xPivot, -yCadera, 0.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mPata));
        m->DrawMatching(shader, patas[p].nombre);
    }
}

// ==============================================================================
// E2: Recorrido Rural (210 a 475 m)
// - Distribución estratificada por franjas (CERO solapamientos)
// - Fauna con ciclo realista: CAMINA SIEMPRE ADELANTE -> SE DETIENE A PASTAR -> GIRA -> CAMINA
// ==============================================================================
void CicloviaScene::DrawRural(Shader& shader, GLint modelLoc) {
    glm::mat4 model(1.0f);
    const float INICIO_E2 = 210.0f;
    const float FIN_BOSQUE = 320.0f;
    const float INICIO_ARIDO = 320.0f;
    const float FIN_ARIDO = 415.0f; // Fin exacto del bioma árido
    const float FIN_E2 = 475.0f; // Cota de 26 m de altura

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
                float dist = 7.5f + azar(sem + 500) * 7.5f;
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
                float dist = 4.2f + azar(sem + 500) * 2.5f;
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
    // -------------------------------------------------------------------------
    int slot = 0;
    for (float s = INICIO_ARIDO + 3.0f; s < FIN_ARIDO; s += 5.5f, slot++) {
        for (int L = 0; L < 2; L++) {
            int sem = slot * 2 + L;
            float signo = (L == 0 ? 1.0f : -1.0f);

            // FRANJA MEDIA (7.0 m a 12.0 m): Huizache o Maguey
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

            // FRANJA CORTA (4.2 m a 6.2 m): Nopal o Tepetate
            if (slot % 2 == 0) {
                if (nopalModel) {
                    float dist = 4.8f + azar(sem + 300) * 1.6f;
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

            // Zacatón de relleno
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
    // 3. FAUNA NATURAL CON CICLO: AVANCE FORWARD -> DETENCIÓN -> GIRO EN LUGAR
    // -------------------------------------------------------------------------
    float t = (float)glfwGetTime();
    const float S_REBANO = 370.0f;
    Ruta::Muestra pMuestra = Ruta::En(S_REBANO);
    float rumboBase = pMuestra.headingDeg;

    // Estructura de cinemática natural determinista
    struct CinemataCuadrupedo {
        glm::vec2 offsetActual;
        float anguloGiroDeg;
        float walkPhase;
        float ampPatas;
        float bobbingY;
        float pastoreoPitch;
    };

    // Función que calcula el comportamiento paso a paso en bucle cerrado
    auto calcularPasoNatural = [&](float tGlobal, float durPaso, float durPausa, float durGiro,
        float velFrec, float radioS, float radioL, float fOffset)
        {
            CinemataCuadrupedo res;
            // 4 waypoints que forman un cuadrilátero de pastoreo seguro
            const glm::vec2 wp[4] = {
                glm::vec2(0.0f,    0.0f),
                glm::vec2(radioS,  radioL),
                glm::vec2(radioS * 0.4f, -radioL * 0.8f),
                glm::vec2(-radioS * 0.7f,  radioL * 0.3f)
            };

            float tSeg = durPaso + durPausa + durGiro;
            float tCiclo = tSeg * 4.0f;
            float tLocal = std::fmod(tGlobal + fOffset, tCiclo);
            if (tLocal < 0.0f) tLocal += tCiclo;

            int tramo = (int)(tLocal / tSeg);
            float tau = tLocal - tramo * tSeg;

            int iSig = (tramo + 1) % 4;
            glm::vec2 A = wp[tramo];
            glm::vec2 B = wp[iSig];
            glm::vec2 delta = B - A;

            // Dirección angular natural hacia donde camina (en grados respecto a la ruta)
            float anguloHaciaB = glm::degrees(std::atan2(-delta.y, delta.x));

            int iSig2 = (tramo + 2) % 4;
            glm::vec2 C = wp[iSig2];
            glm::vec2 deltaSig = C - B;
            float anguloHaciaC = glm::degrees(std::atan2(-deltaSig.y, deltaSig.x));

            if (tau < durPaso) {
                // FASE 1: CAMINATA ESTRICTAMENTE HACIA ADELANTE
                float u = tau / durPaso;
                res.offsetActual = A + delta * u;
                res.anguloGiroDeg = anguloHaciaB;
                res.walkPhase = tau * velFrec;
                res.ampPatas = 22.0f;
                res.bobbingY = std::abs(std::sin(res.walkPhase)) * 0.022f;
                res.pastoreoPitch = 0.0f;
            }
            else if (tau < durPaso + durPausa) {
                // FASE 2: DETENIDO EN EL LUGAR (PASTANDO CON PATAS QUIETAS)
                float tPausa = tau - durPaso;
                res.offsetActual = B;
                res.anguloGiroDeg = anguloHaciaB;
                res.walkPhase = 0.0f;
                res.ampPatas = 0.0f; // Patitas firmes en el suelo sin moverse
                res.bobbingY = 0.0f;
                // Cabeceo lento simulando que come hierba del suelo
                res.pastoreoPitch = std::sin(tPausa * 2.5f) * 4.5f;
            }
            else {
                // FASE 3: GIRO SUAVE EN EL LUGAR HACIA LA NUEVA DIRECCIÓN
                float tGiro = tau - (durPaso + durPausa);
                float uGiro = tGiro / durGiro;
                float factorSuave = uGiro * uGiro * (3.0f - 2.0f * uGiro);

                // Interpolación angular continua
                float diff = anguloHaciaC - anguloHaciaB;
                while (diff > 180.0f)  diff -= 360.0f;
                while (diff < -180.0f) diff += 360.0f;

                res.offsetActual = B;
                res.anguloGiroDeg = anguloHaciaB + diff * factorSuave;
                // Pasitos muy cortos para reacomodar las patas durante el giro
                res.walkPhase = tGiro * 5.0f;
                res.ampPatas = 8.0f;
                res.bobbingY = std::abs(std::sin(res.walkPhase)) * 0.008f;
                res.pastoreoPitch = 0.0f;
            }

            return res;
        };

    // A. CABRAS / CHIVOS (Paso rápido, pausa ágil y giro de exploración)
    if (cabraModel) {
        const struct { float ds; float lateral; float fOff; } cbData[] = {
            { -3.5f,  6.2f, 0.0f },
            {  0.0f,  6.6f, 3.2f },
            {  2.8f,  6.0f, 6.8f },
            { -1.8f, -6.2f, 9.5f }
        };

        for (const auto& cb : cbData) {
            // Camina 2.2 s, se detiene a pastar 2.0 s, gira 0.8 s
            CinemataCuadrupedo c = calcularPasoNatural(t, 2.2f, 2.0f, 0.8f, 7.5f, 1.2f, 0.6f, cb.fOff);

            glm::vec3 p = punto(S_REBANO + cb.ds + c.offsetActual.x, cb.lateral + c.offsetActual.y);
            p.y += c.bobbingY;

            model = glm::mat4(1.0f);
            model = glm::translate(model, p);
            model = glm::rotate(model, glm::radians(rumboBase + c.anguloGiroDeg), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(c.pastoreoPitch), glm::vec3(0.0f, 0.0f, 1.0f)); // Cabeceo

            // Articulación de patas con parámetros exactos de Blender
            DrawCuadrupedo(cabraModel, shader, modelLoc, model, c.walkPhase, 0.24f, -0.26f, 0.63f, c.ampPatas);
        }
    }

    // B. OVEJAS / BORREGOS (Paso calmo, pausas largas de pastoreo en grupo)
    if (ovejaModel) {
        const struct { float ds; float lateral; float fOff; } ovData[] = {
            { -4.0f,  4.5f,  0.5f }, { -2.5f,  5.2f,  3.7f },
            { -1.0f,  4.4f,  6.2f }, {  0.5f,  5.5f,  1.8f },
            {  2.0f,  4.8f,  8.4f }, { -3.0f, -4.8f,  4.9f },
            { -0.5f, -5.5f,  7.1f }, {  1.5f, -4.7f, 10.3f }
        };

        for (const auto& ov : ovData) {
            // Camina 2.5 s, se detiene a pastar 3.0 s, gira 1.0 s
            CinemataCuadrupedo c = calcularPasoNatural(t, 2.5f, 3.0f, 1.0f, 6.2f, 0.9f, 0.45f, ov.fOff);

            glm::vec3 p = punto(S_REBANO + ov.ds + c.offsetActual.x, ov.lateral + c.offsetActual.y);
            p.y += c.bobbingY;

            model = glm::mat4(1.0f);
            model = glm::translate(model, p);
            model = glm::rotate(model, glm::radians(rumboBase + c.anguloGiroDeg), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(c.pastoreoPitch), glm::vec3(0.0f, 0.0f, 1.0f));

            DrawCuadrupedo(ovejaModel, shader, modelLoc, model, c.walkPhase, 0.18f, -0.18f, 0.40f, c.ampPatas);
        }
    }

    // C. CABALLO CASTAÑO (Zancada pausada, se detiene e impone presencia)
    if (caballoModel) {
        // Camina 3.0 s, se detiene calmo 3.5 s, gira 1.2 s
        CinemataCuadrupedo c = calcularPasoNatural(t, 3.0f, 3.5f, 1.2f, 5.0f, 1.6f, 0.7f, 0.0f);

        glm::vec3 p = punto(S_REBANO + 6.5f + c.offsetActual.x, 5.4f + c.offsetActual.y);
        p.y += c.bobbingY;

        model = glm::mat4(1.0f);
        model = glm::translate(model, p);
        model = glm::rotate(model, glm::radians(rumboBase - 20.0f + c.anguloGiroDeg), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(c.pastoreoPitch * 0.4f), glm::vec3(0.0f, 0.0f, 1.0f));

        DrawCuadrupedo(caballoModel, shader, modelLoc, model, c.walkPhase, 0.50f, -0.52f, 1.28f, c.ampPatas);
    }

    // D. PERROS PASTORES (Patrullaje dinámico y perro en guardia vigilante)
    if (perroModel) {
        // Perro 1: Patrullaje activo en forma de romboide
        CinemataCuadrupedo c = calcularPasoNatural(t, 2.0f, 1.2f, 0.6f, 11.0f, 1.5f, 0.8f, 1.5f);

        glm::vec3 pP1 = punto(S_REBANO - 5.5f + c.offsetActual.x, 4.4f + c.offsetActual.y);
        pP1.y += c.bobbingY;

        model = glm::mat4(1.0f);
        model = glm::translate(model, pP1);
        model = glm::rotate(model, glm::radians(rumboBase - 15.0f + c.anguloGiroDeg), glm::vec3(0.0f, 1.0f, 0.0f));

        DrawCuadrupedo(perroModel, shader, modelLoc, model, c.walkPhase, 0.18f, -0.18f, 0.35f, c.ampPatas);

        // Perro 2: En guardia vigilando el camino desde el flanco izquierdo
        glm::vec3 pP2 = punto(S_REBANO + 2.5f, -4.6f);
        float respiroP2 = 1.0f + std::sin(t * 3.5f) * 0.02f;

        model = glm::mat4(1.0f);
        model = glm::translate(model, pP2);
        model = glm::rotate(model, glm::radians(rumboBase + 45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, respiroP2, 1.0f));

        // Patas inmóviles (ampDeg = 0)
        DrawCuadrupedo(perroModel, shader, modelLoc, model, 0.0f, 0.18f, -0.18f, 0.35f, 0.0f);
    }
}