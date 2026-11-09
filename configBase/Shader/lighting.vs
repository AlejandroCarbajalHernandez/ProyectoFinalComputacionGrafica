#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in float boneId;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Animacion por segmentos rigidos (solo el ciclista)
uniform bool useBones;
uniform mat4 bones[10];

void main()
{
    mat4 b = useBones ? bones[int(boneId + 0.5)] : mat4(1.0);
    vec4 p = b * vec4(position, 1.0f);
    gl_Position = projection * view * model * p;
    FragPos = vec3(model * p);
    Normal = mat3(transpose(inverse(model))) * (mat3(b) * normal);
    TexCoords = texCoords;
}
