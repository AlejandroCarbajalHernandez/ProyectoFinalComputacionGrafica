#version 330 core
// Fragment shader básico para renderizado de modelos con textura difusa y recorte alfa
out vec4 FragColor;

// Coordenadas UV interpoladas desde el vertex shader
in vec2 TexCoords;

// Textura difusa principal del material
uniform sampler2D texture_diffuse1;

void main()
{    
    // Muestrea el color de la textura y descarta fragmentos casi transparentes (alpha test)
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    if(texColor.a < 0.1)
        discard;
    FragColor = texColor;
}