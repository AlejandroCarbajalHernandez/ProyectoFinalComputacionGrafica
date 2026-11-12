#version 330 core
// Vertex shader básico para renderizado de modelos estáticos con mapeo de texturas
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// Coordenadas UV enviadas al fragment shader
out vec2 TexCoords;

// Matrices de transformación Modelo-Vista-Proyección
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Reenvía las coordenadas de textura y proyecta el vértice al espacio de recorte
    TexCoords = aTexCoords;    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}