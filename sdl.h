#ifndef SDL_H
#define SDL_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_audio.h>
#include <stdint.h>

#include "chip8.h"


typedef struct {
    SDL_Window   *win;
    SDL_Renderer *ren;
    SDL_Texture  *tex;
    uint32_t on;
    uint32_t off;
} window_t;


static const uint32_t palettes[2][2] = {
    {0xFFFFFFFF, 0x00000000},  /* 0 : black & white          */
    {0xFFFFB000, 0xFF2A1C00}   /* 1 : amber / dark-chocolate */
};

static const SDL_Keycode keymap[16] = {
    SDLK_0,  // 0
    SDLK_1,  // 1
    SDLK_2,  // 2
    SDLK_3,  // 3
    SDLK_4,  // 4
    SDLK_5,  // 5
    SDLK_6,  // 6
    SDLK_7,  // 7
    SDLK_8,  // 8
    SDLK_9,  // 9
    SDLK_A, // A
    SDLK_B, // B
    SDLK_C, // C
    SDLK_D, // D
    SDLK_E, // E
    SDLK_F, // F
};


window_t* sdl_init(int scale);
void sdl_draw(chip8_t *c, window_t *w, int scale);
void sdl_destroy(window_t *w);
void sdl_palette(window_t *w, int idx);


extern int audio_volume;

void sdl_audio_init(void);
void sdl_audio_sound(bool active);
void sdl_audio_destroy(void);

#endif /* SDL_H */
