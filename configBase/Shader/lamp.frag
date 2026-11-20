#version 330 core
// Fragment shader para fuentes de luz emisivas (lámparas/luminarias)
out vec4 FragColor;

void main()
{
    // Asigna un color blanco sólido sin cálculos de iluminación externa
    FragColor = vec4(1.0f);
}