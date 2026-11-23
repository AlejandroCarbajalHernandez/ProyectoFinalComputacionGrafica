// Model.h: un modelo 3D completo, formado por varias piezas.
// Lee el archivo del modelo (.obj), carga sus imágenes y permite dibujarlo entero o solo por partes.
#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "SOIL2/SOIL2.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"

using namespace std;

// Carga una imagen desde el disco y la deja lista para pintar un modelo.
inline GLint TextureFromFile(const char* path, string directory)
{
    string filename = string(path);
    filename = directory + '/' + filename;
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width, height;
    unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
    // Si la imagen se encontró, se prepara para usarse; si no, se avisa en la consola.
    if (image)
    {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        SOIL_free_image_data(image);
    }
    else
    {
        cout << "Texture failed to load at path: " << filename << endl;
        SOIL_free_image_data(image);
    }
    return textureID;
}

// Un modelo 3D: carga todas sus piezas y las dibuja.
class Model
{
public:
    // Crea el modelo leyendo su archivo.
    Model(GLchar* path)
    {
        this->loadModel(path);
    }

    // Dibuja todas las piezas del modelo.
    void Draw(Shader shader)
    {
        for (GLuint i = 0; i < this->meshes.size(); i++)
            this->meshes[i].Draw(shader);
    }

    // Dibuja solo las piezas cuyo material tenga (o no tenga) cierto nombre.
    void DrawMaterialMatching(Shader shader, const string& clave, bool coincide)
    {
        for (GLuint i = 0; i < this->meshes.size(); i++)
        {
            bool c = (this->meshes[i].materialName.find(clave) != string::npos);
            if (c == coincide)
                this->meshes[i].Draw(shader);
        }
    }

    // Dibuja solo las piezas cuyo nombre contenga cierta palabra.
    void DrawMatching(Shader shader, const string& clave)
    {
        for (GLuint i = 0; i < this->meshes.size(); i++)
        {
            if (this->meshes[i].name.find(clave) != string::npos)
                this->meshes[i].Draw(shader);
        }
    }

    // Dibuja todo el modelo menos las piezas cuyo nombre contenga una o dos palabras dadas.
    void DrawExcluding(Shader shader, const string& clave1, const string& clave2 = "")
    {
        for (GLuint i = 0; i < this->meshes.size(); i++)
        {
            bool excluye1 = (this->meshes[i].name.find(clave1) != string::npos);
            bool excluye2 = (!clave2.empty() && this->meshes[i].name.find(clave2) != string::npos);
            if (!excluye1 && !excluye2)
                this->meshes[i].Draw(shader);
        }
    }

    // Cambia el color de las piezas cuyo nombre contenga cierta palabra.
    void SetColorMatching(const string& clave, glm::vec3 color)
    {
        for (GLuint i = 0; i < this->meshes.size(); i++)
        {
            if (this->meshes[i].name.find(clave) != string::npos)
                this->meshes[i].baseColor = color;
        }
    }

    // Cambia qué tanto reflejan el entorno las piezas cuyo material contenga cierta palabra.
    void SetReflectivityMaterial(const string& clave, float refl)
    {
        for (GLuint i = 0; i < this->meshes.size(); i++)
        {
            if (this->meshes[i].materialName.find(clave) != string::npos)
                this->meshes[i].reflectivity = refl;
        }
    }

    // Asigna a cada punto de todas las piezas la parte del cuerpo que le toca, para poder animarlo.
    template<typename Func>
    void AssignBones(Func boneFunc)
    {
        for (GLuint i = 0; i < this->meshes.size(); i++)
        {
            this->meshes[i].AssignBones(boneFunc);
        }
    }

private:
    // Las piezas del modelo, la carpeta donde está su archivo y las imágenes ya cargadas.
    vector<Mesh> meshes;
    string directory;
    vector<Texture> textures_loaded;

    // Lee el archivo del modelo y avisa si hubo un error.
    void loadModel(string path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        this->directory = path.substr(0, path.find_last_of('/'));
        this->processNode(scene->mRootNode, scene);
    }

    // Recorre las partes del modelo y va guardando cada una de sus piezas.
    void processNode(aiNode* node, const aiScene* scene)
    {
        for (GLuint i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            this->meshes.push_back(this->processMesh(mesh, scene));
        }
        for (GLuint i = 0; i < node->mNumChildren; i++)
        {
            this->processNode(node->mChildren[i], scene);
        }
    }

    // Convierte una pieza del archivo en una pieza lista para dibujar: sus puntos, color, material e imágenes.
    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        vector<Vertex> vertices;
        vector<GLuint> indices;
        vector<Texture> textures;
        glm::vec3 baseColor(1.0f);

        // Se leen los puntos de la pieza: posición, hacia dónde mira la superficie y parte de la imagen que le toca.
        for (GLuint i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }

            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertex.BoneID = 0.0f;
            vertices.push_back(vertex);
        }

        // Se lee cómo se unen los puntos para formar triángulos.
        for (GLuint i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (GLuint j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // Se leen el nombre, el color y las imágenes del material de la pieza.
        string materialName = "";
        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiString matName;
            if (material->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS)
            {
                materialName = matName.C_Str();
            }

            aiColor3D color(1.0f, 1.0f, 1.0f);
            if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
            {
                baseColor = glm::vec3(color.r, color.g, color.b);
            }

            vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

            vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        return Mesh(vertices, indices, textures, baseColor, string(mesh->mName.C_Str()), materialName);
    }

    // Carga las imágenes de un material, sin repetir las que ya se cargaron.
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            GLboolean skip = false;
            for (GLuint j = 0; j < textures_loaded.size(); j++)
            {
                // Se revisa si esa imagen ya se había cargado antes.
                if (textures_loaded[j].path == str.C_Str())
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            // Si la imagen es nueva, se carga y se guarda.
            if (!skip)
            {
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                this->textures_loaded.push_back(texture);
            }
        }
        return textures;
    }
};