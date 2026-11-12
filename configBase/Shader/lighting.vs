#version 330 core
// Vertex shader principal: transforma vértices, normales y aplica animación por huesos (skinning rígido)
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in float boneId;

// Salidas interpoladas hacia el fragment shader en espacio mundial
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

// Matrices de transformación Modelo-Vista-Proyección
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Animación por segmentos rígidos (ciclista y aves)
uniform bool useBones;
uniform mat4 bones[10];

void main()
{
    // Selecciona la matriz del hueso asignado al vértice o la matriz identidad si es estático
    mat4 b = useBones ? bones[int(boneId + 0.5)] : mat4(1.0);
    vec4 p = b * vec4(position, 1.0f);

    // Proyección final al espacio de recorte y cálculo de posición/normal en espacio mundial
    gl_Position = projection * view * model * p;
    FragPos = vec3(model * p);
    Normal = mat3(transpose(inverse(model))) * (mat3(b) * normal);
    TexCoords = texCoords;
}