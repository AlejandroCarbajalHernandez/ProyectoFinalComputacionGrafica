#include "PlayerBike.h"
#include "RiderPose.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Ruta.h"

// Constructor: inicializa la posición, orientación, velocidad base y punteros de modelos 3D
PlayerBike::PlayerBike() {
    position = glm::vec3(0.0f, 0.20f, 0.0f);
    rotation = 0.0f;
    speed = 6.0f;           // Velocidad normal (~21 km/h)
    turnSpeed = 65.0f;
    wheelRotation = 0.0f;
    pitch = 0.0f;

    bikeModel = nullptr;
    bikerModel = nullptr;
    helmetModel = nullptr;
    backpackModel = nullptr;
}

// Destructor: libera la memoria asignada a los modelos de la bicicleta, ciclista y accesorios
PlayerBike::~PlayerBike() {
    delete bikeModel;
    delete bikerModel;
    delete helmetModel;
    delete backpackModel;
}

// Carga los archivos .obj del jugador, asigna colores por material y vincula los huesos del ciclista
void PlayerBike::Init() {
    bikeModel = new Model((char*)"../assets/obj/bicicleta/bicicleta.obj");
    bikeModel->SetColorMatching("Cuadro", glm::vec3(0.80f, 0.10f, 0.10f));
    bikeModel->SetColorMatching("Caucho", glm::vec3(0.05f, 0.05f, 0.05f));
    bikeModel->SetColorMatching("Radios", glm::vec3(0.90f, 0.90f, 0.90f));
    bikeModel->SetColorMatching("Maza", glm::vec3(0.60f, 0.60f, 0.65f));
    bikeModel->SetColorMatching("Sillin", glm::vec3(0.10f, 0.10f, 0.10f));
    bikeModel->SetColorMatching("Manubrio", glm::vec3(0.10f, 0.10f, 0.10f));

    bikerModel = new Model((char*)"../assets/obj/ciclista1/modelado-ciclista-blender.obj");
    bikerModel->AssignBones(RiderPose::BoneForVertex);
    helmetModel = new Model((char*)"../assets/obj/casco-ciclista/casco-ciclista.obj");
    backpackModel = new Model((char*)"../assets/obj/equipaje/equipaje.obj");
}

// Procesa entradas de teclado para desplazamiento, giros, atajos de zona y adapta la bicicleta al terreno
void PlayerBike::ProcessInput(bool keys[], float deltaTime) {

    // 1. TELETRANSPORTE POR ZONAS (Pruebas inmediatas)
    auto teletransportarA = [&](float sMetros) {
        Ruta::Muestra p = Ruta::En(sMetros);
        position = p.pos;
        position.y = Ruta::AlturaSuelo(position.x, position.z) + 0.20f;
        rotation = p.headingDeg;
        pitch = 0.0f;
        };

    if (keys[GLFW_KEY_1]) teletransportarA(0.0f);   // E1: Salida Polanco
    if (keys[GLFW_KEY_2]) teletransportarA(215.0f); // E2: Inicio Bosque de Ocotes
    if (keys[GLFW_KEY_3]) teletransportarA(325.0f); // E2: Inicio Bioma Árido / Tepetate
    if (keys[GLFW_KEY_4]) teletransportarA(418.0f); // E2: Inicio de la Terracería
    if (keys[GLFW_KEY_5]) teletransportarA(475.0f); // E3: Llegada a Tepoztlán (cota 26 m)

    // 2. MODIFICADORES DE VELOCIDAD (TURBO)
    float currentSpeed = speed;

    // Shift Izquierdo: Turbo x6 
    if (keys[GLFW_KEY_LEFT_SHIFT]) {
        currentSpeed *= 6.0f;
    }
    // Espacio: Super Turbo x15 (vuelo rápido sobre la pista)
    if (keys[GLFW_KEY_SPACE]) {
        currentSpeed *= 15.0f;
    }

    float velocity = currentSpeed * deltaTime;
    float angularVelocity = turnSpeed * deltaTime;

    // Avanzar (W o Flecha Arriba)
    if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP]) {
        position.x -= sin(glm::radians(rotation)) * velocity;
        position.z -= cos(glm::radians(rotation)) * velocity;
        wheelRotation += (currentSpeed / speed) * 350.0f * deltaTime;
    }
    // Frenar / Reversa (S o Flecha Abajo)
    if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN]) {
        position.x += sin(glm::radians(rotation)) * (velocity * 0.4f);
        position.z += cos(glm::radians(rotation)) * (velocity * 0.4f);
        wheelRotation -= 180.0f * deltaTime;
    }
    // Giro lateral
    if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT]) {
        rotation += angularVelocity;
    }
    if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT]) {
        rotation -= angularVelocity;
    }

    // Adaptación vertical y cabeceo al relieve mediante muestreo delantero y trasero
    glm::vec3 fwd(-sin(glm::radians(rotation)), 0.0f, -cos(glm::radians(rotation)));
    float hF = Ruta::AlturaSuelo(position.x + fwd.x * 0.8f, position.z + fwd.z * 0.8f);
    float hB = Ruta::AlturaSuelo(position.x - fwd.x * 0.8f, position.z - fwd.z * 0.8f);
    position.y = 0.5f * (hF + hB) + 0.20f;
    pitch = glm::degrees(atan2(hF - hB, 1.6f));
}

// Renderizado jerárquico del conjunto bicicleta-ciclista (chasis, ruedas, equipaje, cuerpo y casco)
void PlayerBike::Draw(Shader& shader, GLint modelLoc, bool firstPerson) {
    // 1. Matriz raíz (posición global, rumbo, inclinación del terreno y escala base)
    glm::mat4 root(1.0f);
    root = glm::translate(root, position);
    root = glm::rotate(root, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    root = glm::rotate(root, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    root = glm::scale(root, glm::vec3(0.35f, 0.35f, 0.35f));

    // 2. Chasis de la bicicleta y rotación independiente de las dos ruedas sobre sus ejes
    glm::mat4 modelBike = root;
    modelBike = glm::rotate(modelBike, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBike));
    if (bikeModel) {
        glUniform1f(glGetUniformLocation(shader.Program, "reflectivity"), 0.30f);   // cuadro metálico: Fresnel + cielo
        bikeModel->DrawExcluding(shader, "Trasera", "Delantera");
        glUniform1f(glGetUniformLocation(shader.Program, "reflectivity"), 0.0f);

        float centros[2] = { -0.52f, 0.55f };
        const char* nombres[2] = { "Trasera", "Delantera" };
        for (int w = 0; w < 2; w++) {
            glm::mat4 modelWheel = modelBike;
            modelWheel = glm::translate(modelWheel, glm::vec3(centros[w], 0.0f, 0.0f));
            modelWheel = glm::rotate(modelWheel, glm::radians(-wheelRotation), glm::vec3(0.0f, 0.0f, 1.0f));
            modelWheel = glm::translate(modelWheel, glm::vec3(-centros[w], 0.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelWheel));
            bikeModel->DrawMatching(shader, nombres[w]);
        }
    }

    // 3. Bolsa / Maleta acoplada en la parrilla trasera
    glm::mat4 modelBag = root;
    modelBag = glm::translate(modelBag, glm::vec3(0.0f, 0.95f, -0.60f));
    modelBag = glm::scale(modelBag, glm::vec3(0.50f, 0.50f, 0.50f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBag));
    if (backpackModel) backpackModel->Draw(shader);

    // 4. Ciclista animado por huesos (skinning) y casco acoplado a la matriz de la cabeza
    if (!firstPerson) {
        glm::mat4 bones[RiderPose::BONE_COUNT];
        float pedalAngle = glm::radians(-wheelRotation * 0.35f);
        RiderPose::Compute(pedalAngle, bones);

        glm::mat4 modelBiker = root;
        modelBiker = glm::translate(modelBiker, RiderPose::RIDER_OFFSET);
        modelBiker = glm::rotate(modelBiker, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelBiker));

        glUniform1i(glGetUniformLocation(shader.Program, "useBones"), 1);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "bones"), RiderPose::BONE_COUNT, GL_FALSE, glm::value_ptr(bones[0]));
        if (bikerModel) bikerModel->Draw(shader);
        glUniform1i(glGetUniformLocation(shader.Program, "useBones"), 0);

        // Casco sobre la cabeza (hereda la transformación jerárquica del hueso HEAD)
        glm::mat4 modelHelmet = modelBiker * bones[RiderPose::HEAD];
        modelHelmet = glm::translate(modelHelmet, glm::vec3(0.0f, 1.88f, 0.065f));
        modelHelmet = glm::rotate(modelHelmet, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelHelmet = glm::scale(modelHelmet, glm::vec3(0.115f, 0.21f, 0.105f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelHelmet));
        if (helmetModel) helmetModel->Draw(shader);
    }
}

// Calcula el vector unitario horizontal que apunta hacia el frente de la bicicleta
glm::vec3 PlayerBike::GetFrontVector() {
    return glm::vec3(-sin(glm::radians(rotation)), 0.0f, -cos(glm::radians(rotation)));
}