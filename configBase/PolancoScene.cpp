#include "PolancoScene.h"

#include <fstream>
#include <iostream>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// =====================================================================
//  CONFIGURACION  (ajusta aqui rutas, escalas, rotaciones y posiciones)
// =====================================================================
namespace {

    // ---- Rutas: ../assets/obj/<Carpeta>/<archivo>.obj ----
    // Los archivos de Edificios son los de tu captura. Los demas nombres son suposiciones:
    // si alguno no existe, la consola avisa "[Polanco] AVISO" y se omite ese modelo.
    const char* PATH_MUSEO = "../assets/obj/MuseoS/MuseoS.obj";
    const char* PATH_BANCA = "../assets/obj/Banca/Banca.obj";
    const char* PATH_CONO = "../assets/obj/Cono/Cono.obj";
    const char* PATH_PUENTE = "../assets/obj/PuenteC/PuenteC.obj";
    const char* PATH_VIAS = "../assets/obj/ViasTren/ViasTren.obj";

    const char* PATH_TREES[] = {
        "../assets/obj/ArbolC/ArbolC.obj",
        "../assets/obj/ArbolCT/ArbolCT.obj",
        "../assets/obj/arbol/arbol.obj"
    };
    const char* PATH_LAMPS[] = {
        "../assets/obj/Iluminacion/PIluminacion.obj"
    };
    const char* PATH_CARS[3] = {                       // 0 = CarroA, 1 = CarroB, 2 = AutoBusT
        "../assets/obj/Carror/CarroA.obj",
        "../assets/obj/Carror/CarroB.obj",
        "../assets/obj/Carror/AutoBusT.obj"
    };

    // Orden = indices del enum Bld
    enum Bld { B_A, B_B, B_BEG, B_C, B_T, B_T2, B_V, B_TORRE_CRIS, B_TORRE_CRIS_B, B_COUNT };
    const char* PATH_BUILDINGS[B_COUNT] = {
        "../assets/obj/Edificios/EdificioA.obj",
        "../assets/obj/Edificios/EdificioB.obj",
        "../assets/obj/Edificios/EdificioBeg.obj",
        "../assets/obj/Edificios/EdificioC.obj",
        "../assets/obj/Edificios/EdificioT.obj",
        "../assets/obj/Edificios/EdificioT2.obj",
        "../assets/obj/Edificios/EdificioV.obj",
        "../assets/obj/Edificios/TorreCris.obj",
        "../assets/obj/Edificios/TorreCrisB.obj"
    };

    // ---- Escalas ----
    const float SCALE_MUSEO = 1.0f;
    const float SCALE_BANCA = 0.5f;
    const float SCALE_CONO = 0.12f;
    const float SCALE_PUENTE = 0.22f;
    const float SCALE_VIAS = 0.25f;
    const float SCALE_TREE = 0.6f;
    const float SCALE_LAMP = 0.5f;
    const float SCALE_BUILDING = 1.0f;
    const float SCALE_TORRE = 1.0f;
    const float SCALE_CAR[3] = { 0.5f, 0.5f, 0.6f };

    // ---- Rotaciones base (si algo sale "de lado", cambia estos grados) ----
    const float ROT_CAR_BASE = 0.0f;
    const float ROT_BUILDING_LEFT = 90.0f;    // edificios del lado izquierdo mirando a la avenida
    const float ROT_BUILDING_RIGHT = -90.0f;   // edificios del lado derecho
    const float ROT_BUILDING_BACK = 0.0f;     // edificios del fondo
    const float ROT_BENCH = 90.0f;
    const float ROT_BRIDGE = 0.0f;     // el puente debe cruzar la avenida (eje X)
    const float ROT_VIAS = 0.0f;     // las vias deben correr a lo largo de Z
    const float ROT_MUSEO = 0.0f;

    // ---- Medidas de la escena ----
    const float Z_START = 30.0f;
    const float Z_END = -150.0f;
    const float GROUND_HALF = 220.0f;
    const float CURB_H = 0.12f;

    // Avenida principal (derecha): x de 8 a 20  -> 2 carriles + orilla para la bici
    const float ROAD_X0 = 8.0f, ROAD_X1 = 20.0f;
    // Banqueta con arboles entre las vias y la avenida
    const float STRIP_X0 = 3.5f, STRIP_X1 = 8.0f;
    // Banqueta derecha
    const float RSIDE_X0 = 20.0f, RSIDE_X1 = 25.0f;
    // Corredor de las vias (balasto)
    const float RAIL_X0 = -8.0f, RAIL_X1 = 3.5f;
    const float TRACK_X = 0.0f;
    const float TRACK_SEG_LEN = 20.0f;   // largo del modelo ViasTren (ajusta a tu .obj)
    // Calle lateral izquierda
    const float LROAD_X0 = -22.0f, LROAD_X1 = -8.0f, LROAD_Z_END = -70.0f;

    // Edificio con azotea verde (procedural), entre las vias y el museo
    const float LAWN_X0 = -16.0f, LAWN_X1 = -3.0f;
    const float LAWN_Z0 = -66.0f, LAWN_Z1 = -98.0f;
    const float LAWN_H = 4.0f;

    // Posiciones fijas
    const glm::vec3 POS_MUSEO(-55.0f, 0.0f, -118.0f);   // al fondo a la izquierda
    const glm::vec3 POS_PUENTE(14.0f, 0.0f, -55.0f);    // cruza la avenida

    bool FileExists(const std::string& path) {
        std::ifstream f(path.c_str());
        return f.good();
    }

    struct BuildingSpawn { int model; float x, z, rotY, scale; };
}

// =====================================================================

PolancoScene::PolancoScene() {}

PolancoScene::~PolancoScene() {
    delete museo; delete banca; delete cono; delete puente; delete vias;
    for (auto m : trees) delete m;
    for (auto m : lamps) delete m;
    for (auto m : buildings) delete m;
    for (int i = 0; i < 3; i++) delete cars[i];

    Surface* all[] = { &ground, &road, &leftRoad, &sidewalk, &railBed, &lawnRoof, &whiteWall };
    for (auto s : all) {
        if (s->vao) glDeleteVertexArrays(1, &s->vao);
        if (s->vbo) glDeleteBuffers(1, &s->vbo);
    }
}

Model* PolancoScene::LoadModel(const std::string& path) {
    if (!FileExists(path)) {
        std::cout << "[Polanco] AVISO: no se encontro " << path << " (se omite ese modelo)" << std::endl;
        return nullptr;
    }
    return new Model((char*)path.c_str());
}

// ---------------------------------------------------------------------
//  Geometria plana (pos(3) normal(3) uv(2), igual que CicloviaScene)
// ---------------------------------------------------------------------
void PolancoScene::PushQuad(std::vector<float>& v,
                            glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
                            glm::vec3 n, float uRep, float vRep) {
    auto push = [&](glm::vec3 p, float u, float t) {
        v.insert(v.end(), { p.x, p.y, p.z, n.x, n.y, n.z, u, t });
    };
    push(p0, 0.0f, 0.0f);
    push(p1, uRep, 0.0f);
    push(p2, uRep, vRep);

    push(p2, uRep, vRep);
    push(p3, 0.0f, vRep);
    push(p0, 0.0f, 0.0f);
}

void PolancoScene::BuildSurface(Surface& s, const std::vector<float>& data) {
    s.count = (GLsizei)(data.size() / 8);
    glGenVertexArrays(1, &s.vao);
    glGenBuffers(1, &s.vbo);
    glBindVertexArray(s.vao);
    glBindBuffer(GL_ARRAY_BUFFER, s.vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void PolancoScene::DrawSurface(const Surface& s, GLuint tex) {
    if (tex) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
    }
    glBindVertexArray(s.vao);
    glDrawArrays(GL_TRIANGLES, 0, s.count);
    glBindVertexArray(0);
}

void PolancoScene::DrawModel(Model* m, Shader& shader, GLint modelLoc,
                             const glm::vec3& pos, float rotYDeg, float scale) {
    if (!m) return;
    glm::mat4 model(1.0f);
    model = glm::translate(model, pos);
    model = glm::rotate(model, glm::radians(rotYDeg), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(scale));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    m->Draw(shader);
}

// ---------------------------------------------------------------------
//  Init
// ---------------------------------------------------------------------
void PolancoScene::Init() {
    // ---- Modelos ----
    museo = LoadModel(PATH_MUSEO);
    banca = LoadModel(PATH_BANCA);
    cono = LoadModel(PATH_CONO);
    puente = LoadModel(PATH_PUENTE);
    vias = LoadModel(PATH_VIAS);

    for (const char* p : PATH_TREES) if (Model* m = LoadModel(p)) trees.push_back(m);
    for (const char* p : PATH_LAMPS) if (Model* m = LoadModel(p)) lamps.push_back(m);
    for (int i = 0; i < B_COUNT; i++) buildings.push_back(LoadModel(PATH_BUILDINGS[i]));  // puede haber nullptr
    for (int i = 0; i < 3; i++)       cars[i] = LoadModel(PATH_CARS[i]);

    // ---- Trafico (0 = CarroA, 1 = CarroB, 2 = AutoBusT) ----
    // Avenida: carril x=12.5 va hacia -Z; carril x=17 viene hacia +Z.
    // Calle izquierda: x=-12 (180) y x=-17 (0). La bici usa la orilla x~9.6.
    vehicles = {
        { 0, 12.5f,  -14.0f,   0.0f }, { 1, 12.5f,  -40.0f,   0.0f },
        { 2, 12.5f,  -62.0f,   0.0f }, { 0, 12.5f,  -86.0f,   0.0f },
        { 1, 12.5f, -112.0f,   0.0f }, { 0, 12.5f, -136.0f,   0.0f },
        { 1, 17.0f,   -6.0f, 180.0f }, { 0, 17.0f,  -30.0f, 180.0f },
        { 2, 17.0f,  -52.0f, 180.0f }, { 1, 17.0f,  -76.0f, 180.0f },
        { 0, 17.0f, -100.0f, 180.0f }, { 1, 17.0f, -128.0f, 180.0f },
        { 0, -12.0f,  -8.0f, 180.0f }, { 1, -12.0f, -34.0f, 180.0f },
        { 1, -17.0f, -20.0f,   0.0f }, { 0, -17.0f, -52.0f,   0.0f },
    };

    // ---- Texturas de color solido (1 pixel) para que el suelo se vea a color ----
    // Si luego quieres texturas reales, asigna polanco.roadTex, etc. ANTES de llamar a Init().
    auto solid = [](unsigned char r, unsigned char g, unsigned char b) -> GLuint {
        unsigned char px[3] = { r, g, b };
        GLuint t = 0;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glGenTextures(1, &t);
        glBindTexture(GL_TEXTURE_2D, t);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, px);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        return t;
    };
    if (!groundTex)   groundTex = solid(105, 135, 55);    // pasto
    if (!roadTex)     roadTex = solid(60, 60, 64);      // asfalto
    if (!sidewalkTex) sidewalkTex = solid(175, 172, 160);   // banqueta
    if (!railBedTex)  railBedTex = solid(150, 120, 75);    // tierra seca / balasto
    if (!lawnTex)     lawnTex = solid(60, 140, 50);     // azotea verde
    if (!wallTex)     wallTex = solid(235, 235, 230);   // muros blancos

    const glm::vec3 up(0.0f, 1.0f, 0.0f);
    const float zLen = Z_START - Z_END;

    // Terreno base
    {
        std::vector<float> v;
        PushQuad(v,
            { -GROUND_HALF, -0.05f, Z_START + 40.0f }, { GROUND_HALF, -0.05f, Z_START + 40.0f },
            { GROUND_HALF, -0.05f, Z_END - 60.0f }, { -GROUND_HALF, -0.05f, Z_END - 60.0f },
            up, 80.0f, 80.0f);
        BuildSurface(ground, v);
    }
    // Avenida principal
    {
        std::vector<float> v;
        PushQuad(v, { ROAD_X0, 0.0f, Z_START }, { ROAD_X1, 0.0f, Z_START },
                    { ROAD_X1, 0.0f, Z_END }, { ROAD_X0, 0.0f, Z_END }, up, 2.0f, zLen / 7.0f);
        BuildSurface(road, v);
    }
    // Calle lateral izquierda
    {
        std::vector<float> v;
        float len = Z_START - LROAD_Z_END;
        PushQuad(v, { LROAD_X0, 0.0f, Z_START }, { LROAD_X1, 0.0f, Z_START },
                    { LROAD_X1, 0.0f, LROAD_Z_END }, { LROAD_X0, 0.0f, LROAD_Z_END }, up, 2.0f, len / 7.0f);
        BuildSurface(leftRoad, v);
    }
    // Banquetas: franja entre vias y avenida + banqueta derecha (con guarnicion)
    {
        std::vector<float> v;
        float rep = zLen / 4.0f;
        // franja izquierda de la avenida
        PushQuad(v, { STRIP_X0, CURB_H, Z_START }, { STRIP_X1, CURB_H, Z_START },
                    { STRIP_X1, CURB_H, Z_END }, { STRIP_X0, CURB_H, Z_END }, up, 1.0f, rep);
        PushQuad(v, { STRIP_X1, 0.0f, Z_START }, { STRIP_X1, CURB_H, Z_START },
                    { STRIP_X1, CURB_H, Z_END }, { STRIP_X1, 0.0f, Z_END }, glm::vec3(1, 0, 0), 1.0f, rep);
        // banqueta derecha
        PushQuad(v, { RSIDE_X0, CURB_H, Z_START }, { RSIDE_X1, CURB_H, Z_START },
                    { RSIDE_X1, CURB_H, Z_END }, { RSIDE_X0, CURB_H, Z_END }, up, 1.0f, rep);
        PushQuad(v, { RSIDE_X0, 0.0f, Z_START }, { RSIDE_X0, CURB_H, Z_START },
                    { RSIDE_X0, CURB_H, Z_END }, { RSIDE_X0, 0.0f, Z_END }, glm::vec3(-1, 0, 0), 1.0f, rep);
        BuildSurface(sidewalk, v);
    }
    // Corredor de vias (tierra / balasto)
    {
        std::vector<float> v;
        PushQuad(v, { RAIL_X0, 0.01f, Z_START }, { RAIL_X1, 0.01f, Z_START },
                    { RAIL_X1, 0.01f, Z_END }, { RAIL_X0, 0.01f, Z_END }, up, 3.0f, zLen / 4.0f);
        BuildSurface(railBed, v);
    }
    // Edificio de azotea verde (procedural)
    {
        std::vector<float> roof, walls;
        PushQuad(roof, { LAWN_X0, LAWN_H, LAWN_Z0 }, { LAWN_X1, LAWN_H, LAWN_Z0 },
                       { LAWN_X1, LAWN_H, LAWN_Z1 }, { LAWN_X0, LAWN_H, LAWN_Z1 }, up, 6.0f, 8.0f);
        // frente (+Z), fondo (-Z), izquierda (-X), derecha (+X)
        PushQuad(walls, { LAWN_X0, 0, LAWN_Z0 }, { LAWN_X1, 0, LAWN_Z0 },
                        { LAWN_X1, LAWN_H, LAWN_Z0 }, { LAWN_X0, LAWN_H, LAWN_Z0 }, glm::vec3(0, 0, 1), 3.0f, 1.0f);
        PushQuad(walls, { LAWN_X1, 0, LAWN_Z1 }, { LAWN_X0, 0, LAWN_Z1 },
                        { LAWN_X0, LAWN_H, LAWN_Z1 }, { LAWN_X1, LAWN_H, LAWN_Z1 }, glm::vec3(0, 0, -1), 3.0f, 1.0f);
        PushQuad(walls, { LAWN_X0, 0, LAWN_Z1 }, { LAWN_X0, 0, LAWN_Z0 },
                        { LAWN_X0, LAWN_H, LAWN_Z0 }, { LAWN_X0, LAWN_H, LAWN_Z1 }, glm::vec3(-1, 0, 0), 6.0f, 1.0f);
        PushQuad(walls, { LAWN_X1, 0, LAWN_Z0 }, { LAWN_X1, 0, LAWN_Z1 },
                        { LAWN_X1, LAWN_H, LAWN_Z1 }, { LAWN_X1, LAWN_H, LAWN_Z0 }, glm::vec3(1, 0, 0), 6.0f, 1.0f);
        BuildSurface(lawnRoof, roof);
        BuildSurface(whiteWall, walls);
    }
}

// ---------------------------------------------------------------------
//  Draw
// ---------------------------------------------------------------------
void PolancoScene::Draw(Shader& shader, GLint modelLoc) {
    glm::mat4 model(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // 1. Superficies
    DrawSurface(ground, groundTex);
    DrawSurface(road, roadTex);
    DrawSurface(leftRoad, roadTex);
    DrawSurface(sidewalk, sidewalkTex);
    DrawSurface(railBed, railBedTex);
    DrawSurface(lawnRoof, lawnTex);
    DrawSurface(whiteWall, wallTex);

    // 2. Vias del tren, repetidas a lo largo de la avenida
    if (vias) {
        for (float z = Z_START; z > Z_END; z -= TRACK_SEG_LEN)
            DrawModel(vias, shader, modelLoc, glm::vec3(TRACK_X, 0.02f, z), ROT_VIAS, SCALE_VIAS);
    }

    // 3. Museo Soumaya al fondo a la izquierda
    DrawModel(museo, shader, modelLoc, POS_MUSEO, ROT_MUSEO, SCALE_MUSEO);

    // 4. Edificios (siluetas como en la foto)
    const BuildingSpawn spawns[] = {
        // Torre alta de balcones en primer plano a la izquierda
        { B_A,            -36.0f,   12.0f, ROT_BUILDING_LEFT,  SCALE_BUILDING },
        { B_B,            -36.0f,  -24.0f, ROT_BUILDING_LEFT,  SCALE_BUILDING },
        { B_C,            -36.0f,  -58.0f, ROT_BUILDING_LEFT,  SCALE_BUILDING },
        // Torres de cristal detras del museo
        { B_TORRE_CRIS_B, -40.0f, -185.0f, ROT_BUILDING_BACK,  SCALE_TORRE },
        { B_T,            -85.0f, -175.0f, ROT_BUILDING_BACK,  SCALE_BUILDING },
        // Lado derecho, detras de la hilera de arboles
        { B_V,             42.0f,   10.0f, ROT_BUILDING_RIGHT, SCALE_BUILDING },
        { B_T,             42.0f,  -22.0f, ROT_BUILDING_RIGHT, SCALE_BUILDING },
        { B_T2,            42.0f,  -54.0f, ROT_BUILDING_RIGHT, SCALE_BUILDING },
        { B_BEG,           42.0f,  -86.0f, ROT_BUILDING_RIGHT, SCALE_BUILDING },
        { B_TORRE_CRIS,    42.0f, -122.0f, ROT_BUILDING_RIGHT, SCALE_TORRE },   // torre de cristal de la derecha
        { B_B,             42.0f, -156.0f, ROT_BUILDING_RIGHT, SCALE_BUILDING },
    };
    for (const BuildingSpawn& b : spawns) {
        if (b.model < (int)buildings.size())
            DrawModel(buildings[b.model], shader, modelLoc, glm::vec3(b.x, 0.0f, b.z), b.rotY, b.scale);
    }

    // 5. Arboles: hilera densa a la derecha + franja entre vias y avenida
    if (!trees.empty()) {
        int idx = 0;
        for (float z = Z_START - 4.0f; z > Z_END; z -= 7.0f, idx++) {
            DrawModel(trees[idx % trees.size()], shader, modelLoc,
                      glm::vec3(22.5f, CURB_H, z), 0.0f, SCALE_TREE);
            DrawModel(trees[(idx + 1) % trees.size()], shader, modelLoc,
                      glm::vec3(27.5f, 0.0f, z - 3.0f), 0.0f, SCALE_TREE);
        }
        for (float z = Z_START - 6.0f; z > Z_END; z -= 10.0f, idx++) {
            DrawModel(trees[(idx + 2) % trees.size()], shader, modelLoc,
                      glm::vec3(5.8f, CURB_H, z), 0.0f, SCALE_TREE);
        }
        // Arboles en la calle lateral izquierda
        for (float z = Z_START - 8.0f; z > LROAD_Z_END; z -= 12.0f, idx++) {
            DrawModel(trees[idx % trees.size()], shader, modelLoc,
                      glm::vec3(LROAD_X0 - 1.5f, 0.0f, z), 0.0f, SCALE_TREE);
        }
    }

    // 6. Luminarias a ambos lados de la avenida
    if (!lamps.empty()) {
        int idx = 0;
        for (float z = Z_START - 2.0f; z > Z_END; z -= 16.0f, idx++) {
            DrawModel(lamps[idx % lamps.size()], shader, modelLoc,
                      glm::vec3(ROAD_X0 + 0.4f, CURB_H, z), 0.0f, SCALE_LAMP);
            DrawModel(lamps[idx % lamps.size()], shader, modelLoc,
                      glm::vec3(ROAD_X1 - 0.4f, CURB_H, z - 8.0f), 180.0f, SCALE_LAMP);
        }
    }

    // 7. Bancas en la franja entre vias y avenida
    if (banca) {
        for (float z = Z_START - 14.0f; z > Z_END; z -= 28.0f)
            DrawModel(banca, shader, modelLoc, glm::vec3(4.6f, CURB_H, z), ROT_BENCH, SCALE_BANCA);
    }

    // 8. Conos separando la orilla de la bici
    if (cono) {
        for (float z = -35.0f; z >= -60.0f; z -= 4.0f)
            DrawModel(cono, shader, modelLoc, glm::vec3(10.8f, 0.0f, z), 0.0f, SCALE_CONO);
    }

    // 9. Trafico (incluye el autobus turistico rojo)
    for (const VehicleSpawn& v : vehicles)
        DrawModel(cars[v.kind], shader, modelLoc,
                  glm::vec3(v.x, 0.0f, v.z), v.rotY + ROT_CAR_BASE, SCALE_CAR[v.kind]);

    // 10. Puente peatonal sobre la avenida
    DrawModel(puente, shader, modelLoc, POS_PUENTE, ROT_BRIDGE, SCALE_PUENTE);
}
