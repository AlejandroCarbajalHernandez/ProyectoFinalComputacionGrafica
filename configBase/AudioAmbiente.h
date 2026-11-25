#pragma once
// Audio ambiental por zona de la ruta (libreria miniaudio, un solo archivo .h).
// - Cada zona tiene su sonido de fondo; al cruzar de una zona a otra se mezclan suavemente (crossfade).
// - Efectos esporadicos: ovejas en la zona de ganaderia y campana en Tepoztlan.
// - Si falta un archivo o no hay dispositivo de audio, el juego sigue funcionando sin sonido.
// Archivos esperados en la carpeta "Sonidos" junto al ejecutable (misma logica que la carpeta Shader).
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <algorithm>
#include "miniaudio.h"

namespace Audio {
    namespace {
        enum Capa { CAPA_CIUDAD = 0, CAPA_BOSQUE, CAPA_INSECTOS, CAPA_PUEBLO, NUM_CAPAS };

        struct Fondo {
            ma_sound snd;
            bool cargado = false;
            float vol = 0.0f;        // volumen actual (0 a 1)
        };

        ma_engine motor;
        bool activo = false;
        bool silenciado = false;   // tecla V: silencia todo el audio
        Fondo fondos[NUM_CAPAS];
        ma_sound oveja, campana;
        bool ovejaOK = false, campanaOK = false;
        float tOveja = 6.0f, tCampana = 8.0f;
        std::chrono::steady_clock::time_point ultimo;

        const float VOLUMEN_GENERAL = 0.55f;   // el ambiente no debe tapar la narracion ni el juego
        const float TIEMPO_MEZCLA = 2.5f;      // segundos para pasar de una zona a otra

        inline bool CargarFondo(Fondo& f, const char* ruta) {
            if (ma_sound_init_from_file(&motor, ruta, MA_SOUND_FLAG_STREAM, NULL, NULL, &f.snd) != MA_SUCCESS) {
                std::cout << "[Audio] No se pudo abrir: " << ruta << std::endl;
                return false;
            }
            ma_sound_set_looping(&f.snd, MA_TRUE);
            ma_sound_set_volume(&f.snd, 0.0f);
            f.cargado = true;
            return true;
        }

        inline bool CargarEfecto(ma_sound& s, const char* ruta) {
            if (ma_sound_init_from_file(&motor, ruta, MA_SOUND_FLAG_DECODE, NULL, NULL, &s) != MA_SUCCESS) {
                std::cout << "[Audio] No se pudo abrir: " << ruta << std::endl;
                return false;
            }
            return true;
        }

        inline void Disparar(ma_sound& s, float volumen) {
            ma_sound_set_volume(&s, volumen);
            ma_sound_seek_to_pcm_frame(&s, 0);
            ma_sound_start(&s);
        }
    }

    // Llamar una vez despues de crear la ventana
    inline bool Iniciar() {
        if (ma_engine_init(NULL, &motor) != MA_SUCCESS) {
            std::cout << "[Audio] No hay dispositivo de audio: el juego continua sin sonido." << std::endl;
            return false;
        }
        activo = true;
        CargarFondo(fondos[CAPA_CIUDAD],   "Shader/Sonidos/ciudad.wav");
        CargarFondo(fondos[CAPA_BOSQUE],   "Shader/Sonidos/bosque.mp3");
        CargarFondo(fondos[CAPA_INSECTOS], "Shader/Sonidos/insectos.flac");
        CargarFondo(fondos[CAPA_PUEBLO],   "Shader/Sonidos/pueblo.mp3");
        ovejaOK   = CargarEfecto(oveja,   "Shader/Sonidos/oveja.wav");
        campanaOK = CargarEfecto(campana, "Shader/Sonidos/campana.flac");
        ultimo = std::chrono::steady_clock::now();
        return true;
    }

    // Llamar una vez por cuadro.
    // zona: 0 ciudad, 1 bosque, 2 arido y ganaderia, 3 terraceria, 4 Tepoztlan (-1 = aun sin zona)
    // silencio: true en la bienvenida y el menu. pausa: baja el volumen al pausar.
    inline void Actualizar(int zona, bool silencio, bool pausa) {
        if (!activo) return;

        auto ahora = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(ahora - ultimo).count();
        ultimo = ahora;
        if (dt > 0.25f) dt = 0.25f;   // por si la ventana se congelo un momento

        // Silencio total (tecla V): corta todo de inmediato y no avanza nada mas
        if (silenciado) {
            for (int i = 0; i < NUM_CAPAS; i++) {
                if (!fondos[i].cargado) continue;
                fondos[i].vol = 0.0f;
                ma_sound_set_volume(&fondos[i].snd, 0.0f);
                if (ma_sound_is_playing(&fondos[i].snd) == MA_TRUE) ma_sound_stop(&fondos[i].snd);
            }
            if (ovejaOK)   ma_sound_stop(&oveja);
            if (campanaOK) ma_sound_stop(&campana);
            return;
        }

        if (zona < 0) zona = 0;

        // Volumen objetivo de cada capa segun la zona
        float objetivo[NUM_CAPAS] = { 0.0f, 0.0f, 0.0f, 0.0f };
        switch (zona) {
            case 0: objetivo[CAPA_CIUDAD]   = 1.0f; break;
            case 1: objetivo[CAPA_BOSQUE]   = 1.0f; break;
            case 2: objetivo[CAPA_INSECTOS] = 0.9f; break;
            case 3: objetivo[CAPA_INSECTOS] = 0.6f; break;
            default: objetivo[CAPA_PUEBLO]  = 1.0f; break;
        }
        float factor = silencio ? 0.0f : (pausa ? 0.25f : 1.0f);

        float paso = dt / TIEMPO_MEZCLA;
        for (int i = 0; i < NUM_CAPAS; i++) {
            Fondo& f = fondos[i];
            if (!f.cargado) continue;
            float meta = objetivo[i] * factor;
            if (f.vol < meta) f.vol = std::min(meta, f.vol + paso);
            else              f.vol = std::max(meta, f.vol - paso);

            ma_sound_set_volume(&f.snd, f.vol * VOLUMEN_GENERAL);
            bool suena = ma_sound_is_playing(&f.snd) == MA_TRUE;
            if (f.vol > 0.001f && !suena) ma_sound_start(&f.snd);
            else if (f.vol <= 0.001f && suena) ma_sound_stop(&f.snd);
        }

        // Efectos esporadicos (no suenan en pausa ni en el menu)
        if (silencio || pausa) return;
        if (zona == 2 && ovejaOK) {
            tOveja -= dt;
            if (tOveja <= 0.0f) { Disparar(oveja, 0.7f * VOLUMEN_GENERAL); tOveja = 7.0f + (float)(std::rand() % 8); }
        }
        if (zona == 4 && campanaOK) {
            tCampana -= dt;
            if (tCampana <= 0.0f) { Disparar(campana, 0.8f * VOLUMEN_GENERAL); tCampana = 20.0f + (float)(std::rand() % 15); }
        }
    }

    // Tecla V: activa o desactiva el sonido
    inline void AlternarSilencio() { silenciado = !silenciado; }
    inline bool Silenciado() { return silenciado; }

    // Llamar antes de cerrar el programa
    inline void Fin() {
        if (!activo) return;
        for (int i = 0; i < NUM_CAPAS; i++) if (fondos[i].cargado) ma_sound_uninit(&fondos[i].snd);
        if (ovejaOK)   ma_sound_uninit(&oveja);
        if (campanaOK) ma_sound_uninit(&campana);
        ma_engine_uninit(&motor);
        activo = false;
    }
}
