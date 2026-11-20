#pragma once
// Skybox.h: el cielo que rodea todo el escenario.
// Carga 6 imágenes (una por cada lado de un cubo) y dibuja ese cubo alrededor de la cámara, como si fuera el cielo.
#include <string>
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL2/SOIL2.h"
#include "Shader.h"

// El cielo del escenario: sus imágenes, su forma de cubo y el programa que lo dibuja.
class Skybox {
public:
    // Datos del cielo ya preparados para dibujarse.
    GLuint cubemap = 0;
    GLuint vao = 0, vbo = 0;
    Shader* shader = nullptr;

    // Carga el cielo desde una carpeta que trae seis imágenes: derecha, izquierda, arriba, abajo, frente y atrás.
    bool Init(const std::string& carpeta) {
        shader = new Shader("Shader/skybox.vs", "Shader/skybox.frag");

        // Orden en que OpenGL espera los seis lados del cubo.
        const char* caras[6] = { "derecha", "izquierda", "arriba", "abajo", "frente", "atras" };

        glGenTextures(1, &cubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
        // Se carga cada una de las seis imágenes; si alguna falta, se avisa y se cancela.
        for (int i = 0; i < 6; i++) {
            std::string ruta = carpeta + "/" + caras[i] + ".png";
            int ancho, alto;
            unsigned char* img = SOIL_load_image(ruta.c_str(), &ancho, &alto, 0, SOIL_LOAD_RGB);
            if (!img) {
                std::cout << "Skybox: no se pudo cargar " << ruta << std::endl;
                return false;
            }
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, ancho, alto, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
            SOIL_free_image_data(img);
        }
        // Se suavizan las imágenes y se evita que se noten las uniones entre los lados.
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // Los puntos que forman el cubo del cielo.
        float v[] = {
            -1, 1,-1, -1,-1,-1,  1,-1,-1,  1,-1,-1,  1, 1,-1, -1, 1,-1,
            -1,-1, 1, -1,-1,-1, -1, 1,-1, -1, 1,-1, -1, 1, 1, -1,-1, 1,
             1,-1,-1,  1,-1, 1,  1, 1, 1,  1, 1, 1,  1, 1,-1,  1,-1,-1,
            -1,-1, 1, -1, 1, 1,  1, 1, 1,  1, 1, 1,  1,-1, 1, -1,-1, 1,
            -1, 1,-1,  1, 1,-1,  1, 1, 1,  1, 1, 1, -1, 1, 1, -1, 1,-1,
            -1,-1,-1, -1,-1, 1,  1,-1,-1,  1,-1,-1, -1,-1, 1,  1,-1, 1
        };
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
        return true;
    }

    // Dibuja el cielo antes que el resto de la escena. No sigue los pasos de la cámara, para que siempre se vea lejano.
    void Draw(const glm::mat4& view, const glm::mat4& projection) {
        glDepthFunc(GL_LEQUAL);
        shader->Use();
        glm::mat4 vistaSinMovimiento = glm::mat4(glm::mat3(view));
        glUniformMatrix4fv(glGetUniformLocation(shader->Program, "view"), 1, GL_FALSE, glm::value_ptr(vistaSinMovimiento));
        glUniformMatrix4fv(glGetUniformLocation(shader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(glGetUniformLocation(shader->Program, "skybox"), 0);
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
    }

    // Al terminar se libera de la memoria todo lo del cielo.
    ~Skybox() {
        delete shader;
        if (vao) glDeleteVertexArrays(1, &vao);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (cubemap) glDeleteTextures(1, &cubemap);
    }
};