#pragma once
// Texto.h: escribe texto y dibuja cuadros de color sobre la pantalla (menús, marcadores y minimapa).
// Las letras se arman con pequeños cuadritos, así que no hacen falta librerías ni imágenes. Solo admite mayúsculas,
// números y signos básicos. Las minúsculas se ven como mayúsculas y no se usan acentos (se escribe "CICLOVIA", no "CICLOVÍA").
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <cstdint>

// Todo lo relacionado con escribir en pantalla se agrupa aquí.
namespace Texto {

    // El dibujo de cada letra, número y signo: una cuadrícula de 5 columnas por 7 filas.
    static const uint8_t FUENTE[64][7] = {
        { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, // ' '
        { 0x04,0x04,0x04,0x04,0x04,0x00,0x04 }, // '!'
        { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, // '"'
        { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, // '#'
        { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, // '$'
        { 0x19,0x19,0x02,0x04,0x08,0x13,0x13 }, // '%'
        { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, // '&'
        { 0x04,0x04,0x08,0x00,0x00,0x00,0x00 }, // '''
        { 0x02,0x04,0x08,0x08,0x08,0x04,0x02 }, // '('
        { 0x08,0x04,0x02,0x02,0x02,0x04,0x08 }, // ')'
        { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, // '*'
        { 0x00,0x04,0x04,0x1F,0x04,0x04,0x00 }, // '+'
        { 0x00,0x00,0x00,0x00,0x0C,0x04,0x08 }, // ','
        { 0x00,0x00,0x00,0x1F,0x00,0x00,0x00 }, // '-'
        { 0x00,0x00,0x00,0x00,0x00,0x0C,0x0C }, // '.'
        { 0x01,0x01,0x02,0x04,0x08,0x10,0x10 }, // '/'
        { 0x0E,0x11,0x13,0x15,0x19,0x11,0x0E }, // '0'
        { 0x04,0x0C,0x04,0x04,0x04,0x04,0x0E }, // '1'
        { 0x0E,0x11,0x01,0x02,0x04,0x08,0x1F }, // '2'
        { 0x1E,0x01,0x01,0x0E,0x01,0x01,0x1E }, // '3'
        { 0x02,0x06,0x0A,0x12,0x1F,0x02,0x02 }, // '4'
        { 0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E }, // '5'
        { 0x0E,0x10,0x10,0x1E,0x11,0x11,0x0E }, // '6'
        { 0x1F,0x01,0x02,0x04,0x08,0x08,0x08 }, // '7'
        { 0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E }, // '8'
        { 0x0E,0x11,0x11,0x0F,0x01,0x01,0x0E }, // '9'
        { 0x00,0x0C,0x0C,0x00,0x0C,0x0C,0x00 }, // ':'
        { 0x00,0x0C,0x0C,0x00,0x0C,0x04,0x08 }, // ';'
        { 0x02,0x04,0x08,0x10,0x08,0x04,0x02 }, // '<'
        { 0x00,0x00,0x1F,0x00,0x1F,0x00,0x00 }, // '='
        { 0x08,0x04,0x02,0x01,0x02,0x04,0x08 }, // '>'
        { 0x0E,0x11,0x01,0x02,0x04,0x00,0x04 }, // '?'
        { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, // '@'
        { 0x0E,0x11,0x11,0x1F,0x11,0x11,0x11 }, // 'A'
        { 0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E }, // 'B'
        { 0x0F,0x10,0x10,0x10,0x10,0x10,0x0F }, // 'C'
        { 0x1E,0x11,0x11,0x11,0x11,0x11,0x1E }, // 'D'
        { 0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F }, // 'E'
        { 0x1F,0x10,0x10,0x1E,0x10,0x10,0x10 }, // 'F'
        { 0x0F,0x10,0x10,0x13,0x11,0x11,0x0E }, // 'G'
        { 0x11,0x11,0x11,0x1F,0x11,0x11,0x11 }, // 'H'
        { 0x0E,0x04,0x04,0x04,0x04,0x04,0x0E }, // 'I'
        { 0x07,0x02,0x02,0x02,0x02,0x12,0x0C }, // 'J'
        { 0x11,0x12,0x14,0x18,0x14,0x12,0x11 }, // 'K'
        { 0x10,0x10,0x10,0x10,0x10,0x10,0x1F }, // 'L'
        { 0x11,0x1B,0x15,0x15,0x11,0x11,0x11 }, // 'M'
        { 0x11,0x19,0x15,0x13,0x11,0x11,0x11 }, // 'N'
        { 0x0E,0x11,0x11,0x11,0x11,0x11,0x0E }, // 'O'
        { 0x1E,0x11,0x11,0x1E,0x10,0x10,0x10 }, // 'P'
        { 0x0E,0x11,0x11,0x11,0x15,0x12,0x0D }, // 'Q'
        { 0x1E,0x11,0x11,0x1E,0x14,0x12,0x11 }, // 'R'
        { 0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E }, // 'S'
        { 0x1F,0x04,0x04,0x04,0x04,0x04,0x04 }, // 'T'
        { 0x11,0x11,0x11,0x11,0x11,0x11,0x0E }, // 'U'
        { 0x11,0x11,0x11,0x11,0x11,0x0A,0x04 }, // 'V'
        { 0x11,0x11,0x11,0x15,0x15,0x1B,0x11 }, // 'W'
        { 0x11,0x11,0x0A,0x04,0x0A,0x11,0x11 }, // 'X'
        { 0x11,0x11,0x0A,0x04,0x04,0x04,0x04 }, // 'Y'
        { 0x1F,0x01,0x02,0x04,0x08,0x10,0x1F }, // 'Z'
        { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, // '['
        { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, // 'backslash'
        { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, // ']'
        { 0x00,0x00,0x00,0x00,0x00,0x00,0x00 }, // '^'
        { 0x00,0x00,0x00,0x00,0x00,0x00,0x1F }, // '_'
    };

    // El programa de dibujo y los datos que usa para pintar el texto y los cuadros.
    static GLuint programa = 0, vao = 0, vbo = 0;

    // Prepara el programa de dibujo que se usa para el texto y los cuadros; se llama una vez al iniciar.
    inline void Init() {
        const char* vs =
            "#version 330 core\n"
            "layout (location = 0) in vec2 pos;\n"
            "uniform mat4 projection;\n"
            "void main() { gl_Position = projection * vec4(pos, 0.0, 1.0); }\0";
        const char* fs =
            "#version 330 core\n"
            "out vec4 color;\n"
            "uniform vec4 uiColor;\n"
            "void main() { color = uiColor; }\0";
        GLuint v = glCreateShader(GL_VERTEX_SHADER);   glShaderSource(v, 1, &vs, NULL); glCompileShader(v);
        GLuint f = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(f, 1, &fs, NULL); glCompileShader(f);
        programa = glCreateProgram();
        glAttachShader(programa, v); glAttachShader(programa, f); glLinkProgram(programa);
        glDeleteShader(v); glDeleteShader(f);

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glBindVertexArray(0);
    }

    // Libera de la memoria lo que se preparó; se llama al cerrar el programa.
    inline void Fin() {
        if (vao) glDeleteVertexArrays(1, &vao);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (programa) glDeleteProgram(programa);
    }

    // Calcula cuánto mide de ancho un texto en pantalla, según su tamaño.
    inline float Ancho(const std::string& s, float escala) { return s.size() * 6.0f * escala; }

    // Escribe un texto en la posición indicada de la pantalla, con su tamaño y color, y opcionalmente con una sombra para que se lea mejor.
    inline void Dibujar(const std::string& texto, float x, float y, float escala,
        glm::vec4 color, int anchoPantalla, int altoPantalla, bool sombra = true)
    {
        // Aquí se van juntando los cuadritos que forman las letras.
        std::vector<float> v;
        v.reserve(texto.size() * 35 * 12);
        auto pixel = [&](float px, float py) {
            float x1 = px + escala, y1 = py + escala;
            float q[12] = { px, py, x1, py, x1, y1, px, py, x1, y1, px, y1 };
            v.insert(v.end(), q, q + 12);
            };
        // Se recorre cada letra del texto y se dibuja con sus cuadritos, una junto a otra.
        float cx = 0.0f;
        for (char ch : texto) {
            // Las minúsculas se convierten a mayúsculas.
            if (ch >= 'a' && ch <= 'z') ch = ch - 'a' + 'A';
            int idx = ch - 32;
            // Si el carácter no existe en la fuente, se muestra un espacio.
            if (idx < 0 || idx > 63) idx = 0;
            for (int fila = 0; fila < 7; fila++)
                for (int col = 0; col < 5; col++)
                    if (FUENTE[idx][fila] & (1 << (4 - col)))
                        pixel(cx + col * escala, (6 - fila) * escala);
            cx += 6.0f * escala;
        }
        if (v.empty()) return;

        glUseProgram(programa);
        glm::mat4 proj = glm::ortho(0.0f, (float)anchoPantalla, 0.0f, (float)altoPantalla);
        glUniformMatrix4fv(glGetUniformLocation(programa, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // Dibuja el texto una vez, con el color y el desplazamiento indicados.
        auto pasada = [&](float dx, float dy, glm::vec4 c) {
            std::vector<float> w(v);
            for (size_t i = 0; i < w.size(); i += 2) { w[i] += x + dx; w[i + 1] += y + dy; }
            glBufferData(GL_ARRAY_BUFFER, w.size() * sizeof(float), w.data(), GL_STREAM_DRAW);
            glUniform4fv(glGetUniformLocation(programa, "uiColor"), 1, glm::value_ptr(c));
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(w.size() / 2));
            };
        // Primero se dibuja la sombra (negra y un poco desplazada) y encima el texto.
        if (sombra) pasada(escala * 0.8f, -escala * 0.8f, glm::vec4(0.0f, 0.0f, 0.0f, 0.75f * color.a));
        pasada(0.0f, 0.0f, color);
        glBindVertexArray(0);
    }

    // Dibuja un rectángulo de color, por ejemplo el fondo de un panel.
    inline void Panel(float x, float y, float w, float h, glm::vec4 color, int anchoPantalla, int altoPantalla) {
        float q[12] = { x, y, x + w, y, x + w, y + h, x, y, x + w, y + h, x, y + h };
        glUseProgram(programa);
        glm::mat4 proj = glm::ortho(0.0f, (float)anchoPantalla, 0.0f, (float)altoPantalla);
        glUniformMatrix4fv(glGetUniformLocation(programa, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform4fv(glGetUniformLocation(programa, "uiColor"), 1, glm::value_ptr(color));
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(q), q, GL_STREAM_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    // Dibuja muchos cuadritos del mismo color de una sola vez (se usa para el minimapa).
    // Cada punto marca el centro de un cuadrito, y el lado indica su tamaño.
    inline void Cuadros(const std::vector<glm::vec2>& puntos, float lado, glm::vec4 color,
        int anchoPantalla, int altoPantalla)
    {
        if (puntos.empty()) return;
        std::vector<float> v;
        v.reserve(puntos.size() * 12);
        float h = lado * 0.5f;
        for (const glm::vec2& p : puntos) {
            float x0 = p.x - h, y0 = p.y - h, x1 = p.x + h, y1 = p.y + h;
            float q[12] = { x0, y0, x1, y0, x1, y1, x0, y0, x1, y1, x0, y1 };
            v.insert(v.end(), q, q + 12);
        }
        glUseProgram(programa);
        glm::mat4 proj = glm::ortho(0.0f, (float)anchoPantalla, 0.0f, (float)altoPantalla);
        glUniformMatrix4fv(glGetUniformLocation(programa, "projection"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform4fv(glGetUniformLocation(programa, "uiColor"), 1, glm::value_ptr(color));
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STREAM_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(v.size() / 2));
        glBindVertexArray(0);
    }

} // namespace Texto