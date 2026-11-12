#version 330 core
// Vertex shader para fuentes de luz emisivas (lámparas/luminarias)
layout (location = 0) in vec3 position;

// Matrices de transformación del espacio local al espacio de recorte
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Proyecta la posición del vértice aplicando las matrices Modelo-Vista-Proyección
    gl_Position = projection * view * model * vec4(position, 1.0f);
}