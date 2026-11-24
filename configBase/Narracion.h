// Narracion.h: cajas de texto informativas (indicaciones y datos interesantes) que aparecen al avanzar por la ruta.
// Se usa con Texto.h (Texto::Panel, Texto::Dibujar y Texto::Ancho), igual que el resto de la interfaz.
#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include "Texto.h"

// Texto.h ya dibuja Á É Í Ó Ú Ü Ñ ¿ ¡, así que se deja en 1. Si algún día usas una Texto.h sin acentos, ponlo en 0
// y los acentos se quitan solos para que no salgan letras raras.
#define TEXTO_CON_ACENTOS 1

namespace Narracion {

    // Convierte un texto (guardado como UTF-8 o como ANSI) al formato de Texto: todo en mayúsculas y,
    // según TEXTO_CON_ACENTOS, con las letras acentuadas (UTF-8) o sin acento.
    inline std::string Preparar(const std::string& in) {
        std::string out;
        size_t i = 0;
        while (i < in.size()) {
            unsigned char c = (unsigned char)in[i];
            unsigned cp = c;
            size_t len = 1;
            if (c >= 0xC2 && c <= 0xDF && i + 1 < in.size() && (((unsigned char)in[i + 1]) & 0xC0) == 0x80) {
                cp = ((c & 0x1Fu) << 6) | (((unsigned char)in[i + 1]) & 0x3Fu);   // UTF-8 de 2 bytes
                len = 2;
            }
            else if (c >= 0xE2 && c <= 0xEF && i + 2 < in.size()
                && (((unsigned char)in[i + 1]) & 0xC0) == 0x80 && (((unsigned char)in[i + 2]) & 0xC0) == 0x80) {
                cp = '-';                                                          // comillas, guiones largos, etc.
                len = 3;
            }
            i += len;

            if (cp >= 'a' && cp <= 'z') cp -= 32;                                  // minúscula -> mayúscula
            else if (cp >= 0xE0 && cp <= 0xFE && cp != 0xF7) cp -= 0x20;           // à..þ -> À..Þ

            if (cp < 0x80) { out += (char)cp; continue; }

#if TEXTO_CON_ACENTOS
            switch (cp) {
            case 0xC1: case 0xC9: case 0xCD: case 0xD3: case 0xDA:
            case 0xD1: case 0xDC: case 0xBF: case 0xA1:
                out += (char)(0xC0 | (cp >> 6));       // se escribe como UTF-8 (2 bytes), que Texto.h entiende
                out += (char)(0x80 | (cp & 0x3F));
                break;
            default: out += '?'; break;
            }
#else
            switch (cp) {
            case 0xC1: case 0xC0: case 0xC2: case 0xC4: out += 'A'; break;
            case 0xC9: case 0xC8: case 0xCA: case 0xCB: out += 'E'; break;
            case 0xCD: case 0xCC: case 0xCE: case 0xCF: out += 'I'; break;
            case 0xD3: case 0xD2: case 0xD4: case 0xD6: out += 'O'; break;
            case 0xDA: case 0xD9: case 0xDB: case 0xDC: out += 'U'; break;
            case 0xD1: out += 'N'; break;
            case 0xBF: case 0xA1: break;          // ¿ y ¡ se omiten si la fuente no los tiene
            default: out += '?'; break;
            }
#endif
        }
        return out;
    }

    // Parte un texto en renglones que no pasen de "maxAncho" píxeles. Un '\n' fuerza un renglón nuevo.
    inline std::vector<std::string> Ajustar(const std::string& txt, float maxAncho, float esc) {
        std::vector<std::string> lineas;
        size_t ini = 0;
        while (ini <= txt.size()) {
            size_t fin = txt.find('\n', ini);
            std::string parrafo = txt.substr(ini, fin == std::string::npos ? std::string::npos : fin - ini);
            std::string actual;
            size_t p = 0;
            while (p < parrafo.size()) {
                size_t e = parrafo.find(' ', p);
                std::string palabra = parrafo.substr(p, e == std::string::npos ? std::string::npos : e - p);
                std::string prueba = actual.empty() ? palabra : actual + " " + palabra;
                if (!actual.empty() && Texto::Ancho(prueba, esc) > maxAncho) {
                    lineas.push_back(actual);
                    actual = palabra;
                }
                else {
                    actual = prueba;
                }
                if (e == std::string::npos) break;
                p = e + 1;
            }
            lineas.push_back(actual);
            if (fin == std::string::npos) break;
            ini = fin + 1;
        }
        return lineas;
    }

    // Una caja de información: aparece cuando el ciclista llega a la distancia "s" de la ruta.
    struct Info {
        float s;               // metros de ruta donde aparece
        float duracion;        // segundos en pantalla
        const char* titulo;
        const char* texto;     // '\n' separa párrafos
    };

    // IMPORTANTE: ajusta las "s" al lugar real de cada cosa en tu ruta (la de Soumaya sobre todo).
    inline const std::vector<Info>& Lista() {
        static const std::vector<Info> L = {
            { 0.0f, 8.0f, "INDICACIONES",
              "BOTELLAS DE AGUA: recógelas pasando sobre ellas. Cada una recupera 25 % de tu hidratación. Si llegas a 0 %, pierdes la partida.\n"
              "AIRE: la presión de tus llantas se mide en PSI (libras por pulgada cuadrada) y empiezas con 35. Baja si te sales de la pista y en la terracería, y si llega a 0 pierdes. Para inflarlas, frena casi por completo junto a una bomba de aire.\n"
              "CLAVOS: esquívalos. Cada uno te quita 12 PSI de presión." },
            { 26.0f, 12.0f, "MUSEO SOUMAYA",
              "El Museo Soumaya tiene una fachada de 16 mil hexágonos de aluminio y mide 47 metros de altura. Lo diseñó Fernando Romero, se inauguró en 2011 y su entrada es gratuita." },
            { 112.0f, 10.0f, "FERROCARRIL DE CUERNAVACA",
              "A tu izquierda están las vías y un tren. Esta ciclovía sigue casi la ruta del antiguo Ferrocarril de Cuernavaca: su estación se construyó en 1897 y la ciclovía se inauguró en 2011." },
            { 212.0f, 8.0f, "EMILIANO ZAPATA",
              "A tu izquierda está la estatua de Emiliano Zapata (1879-1919), líder campesino de Morelos y jefe del Ejército Libertador del Sur.\n"
              "En 1911 proclamó el Plan de Ayala, con el lema \"Tierra y Libertad\". Fue asesinado en Chinameca el 10 de abril de 1919." },
            { 240.0f, 8.0f, "BOSQUE DE OCOTES",
              "El ocote (Pinus montezumae) crece entre 2,000 y 3,200 metros de altura y su madera resinosa se ha usado para encender fuego. Esta región forma parte del corredor biológico Chichinautzin.\n"
              "Aquí el aire es fresco, por eso te deshidratas más despacio." },
            { 268.0f, 9.0f, "PUENTE RURAL",
              "Estás por llegar al puente rojo con domo y a los letreros de la Ciclovía Rural CDMX.\n"
              "¡Es un gran spot para sacarte fotos! Aprovecha para mirar el bosque a tu alrededor." },
            { 322.0f, 8.0f, "BIOMA ÁRIDO",
              "El suelo es seco y duro (tepetate) y crecen nopales, magueyes y huizaches. Los nopales guardan agua en sus pencas.\n"
              "Hace calor: te deshidratas mucho más rápido. ¡Aprovecha las botellas!" },
            { 350.0f, 12.0f, "ZONA DE GANADERÍA",
              "Delante de ti pastan ovejas, cabras y caballos. Aquí se cría el borrego para la barbacoa, que se cocina en un hoyo bajo tierra envuelta en pencas de maguey." },
            { 417.0f, 11.0f, "TERRACERÍA",
              "Es un camino sin pavimento: las piedras y el polvo desgastan más tus llantas, y el calor sigue gastando tu agua. Busca las bombas de aire." },
            { 477.0f, 11.0f, "LLEGANDO A TEPOZTLÁN",
              "En lo alto del cerro del Tepozteco hay un templo prehispánico dedicado a Tepoztécatl, deidad del pulque. El cerro está dentro del Parque Nacional El Tepozteco.\n"
              "¡Ya casi llegas a la plaza!" },
        };
        return L;
    }

    // Estado de las cajas (qué caja se está mostrando y cuáles ya se vieron).
    struct Estado {
        std::vector<bool> visto;
        int activa = -1;
        float timer = 0.0f;
        float dur = 0.0f;      // duración de la caja activa
    };
    inline Estado& Est() { static Estado e; return e; }

    // Se llama al reiniciar la partida.
    inline void Reiniciar() {
        Estado& e = Est();
        e.visto.assign(Lista().size(), false);
        e.activa = -1;
        e.timer = 0.0f;
    }

    // Se llama cada cuadro mientras se juega. Activa la caja que toque y descuenta el tiempo.
    inline void Actualizar(float sJugador, float dt) {
        Estado& e = Est();
        if (e.visto.size() != Lista().size()) Reiniciar();

        if (e.timer > 0.0f) e.timer -= dt;
        if (e.timer <= 0.0f) e.activa = -1;

        for (size_t i = 0; i < Lista().size(); i++) {
            if (e.visto[i]) continue;
            const Info& in = Lista()[i];
            if (sJugador >= in.s + 80.0f) { e.visto[i] = true; continue; }   // se pasó de largo (salto de zona): no se muestra
            if (sJugador >= in.s && (e.activa < 0 || (e.dur - e.timer) >= 6.0f)) {   // si hay otra caja, espera a que lleve 6 s en pantalla y la reemplaza
                e.visto[i] = true;
                e.activa = (int)i;
                e.timer = in.duracion;
                e.dur = in.duracion;
            }
        }
    }

    // Dibuja la caja activa abajo al centro, con aparición y desvanecimiento suaves.
    inline void Dibujar(int W, int H) {
        Estado& e = Est();
        if (e.activa < 0 || e.timer <= 0.0f) return;
        const Info& in = Lista()[e.activa];

        float transcurrido = in.duracion - e.timer;
        float a = std::min(std::min(1.0f, transcurrido / 0.4f), std::min(1.0f, e.timer / 0.6f));

        const float ESC = 2.0f, ALTO_RENGLON = 26.0f, PAD = 14.0f;
        // La caja va abajo al centro, pero siempre a la derecha de las barras de PSI e hidratación (terminan en x = 255).
        float x = std::max((W - std::min(0.60f * W, 800.0f)) * 0.5f, 285.0f);
        float ancho = std::min(800.0f, W - x - 20.0f);
        std::vector<std::string> lineas = Ajustar(Preparar(in.texto), ancho - 2.0f * PAD, ESC);

        float alto = 2.0f * PAD + 51.0f + (float)(lineas.size() - 1) * ALTO_RENGLON;
        float y = 24.0f;

        Texto::Panel(x, y, ancho, alto, glm::vec4(0.0f, 0.0f, 0.0f, 0.60f * a), W, H);
        float yTitulo = y + alto - PAD - 21.0f;
        Texto::Dibujar(Preparar(in.titulo), x + PAD, yTitulo, 3.0f, glm::vec4(1.0f, 0.85f, 0.2f, a), W, H);
        float yl = yTitulo - 30.0f;
        for (const std::string& l : lineas) {
            Texto::Dibujar(l, x + PAD, yl, ESC, glm::vec4(1.0f, 1.0f, 1.0f, a), W, H);
            yl -= ALTO_RENGLON;
        }
    }

} // namespace Narracion
