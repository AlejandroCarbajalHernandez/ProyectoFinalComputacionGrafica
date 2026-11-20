// Mesh.h: una pieza de un modelo 3D.
// Guarda sus puntos, sus imágenes y su color, y sabe dibujarse en pantalla.
#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

using namespace std;

// Un punto del modelo: dónde está, hacia dónde mira su superficie, qué parte de la imagen le toca y a qué parte del cuerpo pertenece.
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    float BoneID = 0.0f; // Coincide con layout (location = 3) in float boneId
};

// Una imagen que se usa para pintar la pieza.
struct Texture {
    GLuint id;
    string type;
    string path;
};

// Una pieza del modelo, lista para dibujarse.
class Mesh {
public:
    // Los puntos de la pieza, el orden en que se unen para formar triángulos y sus imágenes.
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;
    // Color liso, nombre de la pieza y del material, y qué tanto refleja el entorno.
    glm::vec3 baseColor;
    string name;
    string materialName;
    float reflectivity = 0.0f;

    // Crea la pieza con sus datos y la deja lista para dibujarse.
    Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures,
        glm::vec3 baseColor = glm::vec3(1.0f), string name = "", string materialName = "")
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        this->baseColor = baseColor;
        this->name = name;
        this->materialName = materialName;

        this->setupMesh();
    }

    // Dibuja la pieza en pantalla.
    void Draw(Shader shader)
    {
        // Contadores para numerar las imágenes de la pieza.
        GLuint diffuseNr = 1;
        GLuint specularNr = 1;

        if (this->textures.empty())
        {
            // Si la pieza no tiene imagen, se pinta de un solo color (bicicleta, ciclista, piezas lisas).
            glUniform1i(glGetUniformLocation(shader.Program, "useColor"), 1);
            glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), this->baseColor.r, this->baseColor.g, this->baseColor.b);
        }
        else
        {
            // Si la pieza tiene imagen, se usa para pintarla.
            glUniform1i(glGetUniformLocation(shader.Program, "useColor"), 0);

            for (GLuint i = 0; i < this->textures.size(); i++)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                stringstream ss;
                string number;
                string tName = this->textures[i].type;
                if (tName == "texture_diffuse")
                    ss << diffuseNr++;
                else if (tName == "texture_specular")
                    ss << specularNr++;
                number = ss.str();

                glUniform1i(glGetUniformLocation(shader.Program, (tName + number).c_str()), i);
                glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
            }
        }

        // Se indica qué tan brillante y qué tan reflejante es la pieza.
        glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 16.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "materialReflect"), this->reflectivity);

        // Se dibuja la pieza.
        glBindVertexArray(this->VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)this->indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Al terminar se sueltan las imágenes para que no afecten a otras piezas.
        if (!this->textures.empty())
        {
            for (GLuint i = 0; i < this->textures.size(); i++)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }

    // Decide a qué parte del cuerpo (ala, pierna, brazo, etc.) pertenece cada punto del modelo, para poder animarlo.
    template<typename Func>
    void AssignBones(Func boneFunc)
    {
        glBindVertexArray(this->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

        for (size_t i = 0; i < this->vertices.size(); i++)
        {
            this->vertices[i].BoneID = static_cast<float>(boneFunc(this->vertices[i].Position));
        }

        // Se vuelven a guardar los datos del modelo con esa información nueva.
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

        // Se indica cómo leer la parte del cuerpo de cada punto.
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BoneID));

        glBindVertexArray(0);
    }

private:
    // Identificadores internos donde OpenGL guarda los datos de la pieza.
    GLuint VAO, VBO, EBO;

    // Envía los datos de la pieza a la tarjeta gráfica para poder dibujarla.
    void setupMesh()
    {
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);

        glBindVertexArray(this->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

        // Se indica cómo leer la posición de cada punto.
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // Se indica cómo leer hacia dónde mira la superficie en cada punto (para la luz).
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        // Se indica cómo leer qué parte de la imagen le toca a cada punto.
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        // Se indica cómo leer la parte del cuerpo de cada punto (empieza en 0 por defecto).
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BoneID));

        glBindVertexArray(0);
    }
};