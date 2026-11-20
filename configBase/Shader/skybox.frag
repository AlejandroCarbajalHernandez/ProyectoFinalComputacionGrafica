#version 330 core
in vec3 TexCoords;
out vec4 color;

uniform samplerCube skybox;
uniform float brillo; // 1 = día, 0 = noche

void main()
{
    vec3 cielo = texture(skybox, TexCoords).rgb;
    vec3 noche = cielo * vec3(0.06, 0.08, 0.16); // cielo oscuro azulado
    color = vec4(mix(noche, cielo, brillo), 1.0);
}