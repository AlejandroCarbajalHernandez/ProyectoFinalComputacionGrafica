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

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    float BoneID = 0.0f; // Coincide con layout (location = 3) in float boneId
};

struct Texture {
    GLuint id;
    string type;
    string path;
};

class Mesh {
public:
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;
    glm::vec3 baseColor;
    string name;
    string materialName;
    float reflectivity = 0.0f;

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

    void Draw(Shader shader)
    {
        GLuint diffuseNr = 1;
        GLuint specularNr = 1;

        if (this->textures.empty())
        {
            // Malla sin textura (bicicleta, ciclista, piezas de color solido):
            glUniform1i(glGetUniformLocation(shader.Program, "useColor"), 1);
            glUniform3f(glGetUniformLocation(shader.Program, "meshColor"), this->baseColor.r, this->baseColor.g, this->baseColor.b);
        }
        else
        {
            // Malla con textura real de imagen:
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

        glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 16.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "materialReflect"), this->reflectivity);

        glBindVertexArray(this->VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)this->indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        if (!this->textures.empty())
        {
            for (GLuint i = 0; i < this->textures.size(); i++)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }

    // Evalúa cada vértice espacialmente con la función de pesaje (AvesPose o RiderPose)
    template<typename Func>
    void AssignBones(Func boneFunc)
    {
        glBindVertexArray(this->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

        for (size_t i = 0; i < this->vertices.size(); i++)
        {
            this->vertices[i].BoneID = static_cast<float>(boneFunc(this->vertices[i].Position));
        }

        // Reescribe el VBO con el nuevo campo BoneID actualizado
        glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

        // Location 3: in float boneId
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BoneID));

        glBindVertexArray(0);
    }

private:
    GLuint VAO, VBO, EBO;

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

        // Location 0: position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // Location 1: normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        // Location 2: texCoords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        // Location 3: boneId (inicia en 0.0f por defecto)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BoneID));

        glBindVertexArray(0);
    }
};