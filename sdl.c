#include "sdl.h"

/* Init / Destroy */
window_t* sdl_init(int scale)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return NULL;
    }

    window_t *w = SDL_malloc(sizeof(*w));
    w->win  = SDL_CreateWindow("MyChip8", 64 * scale, 32 * scale, 0);
    w->ren  = SDL_CreateRenderer(w->win, NULL);
    SDL_SetRenderVSync(w->ren, 1);
    w->on   = palettes[0][0];
    w->off  = palettes[0][1];
    return w;
}

void sdl_destroy(window_t *w)
{
    if (!w) return;
    SDL_DestroyRenderer(w->ren);
    SDL_DestroyWindow(w->win);
    SDL_free(w);
    SDL_Quit();
}

void sdl_draw(chip8_t *c, window_t *w, int scale)
{
    SDL_FRect r = {0, 0, scale, scale};

    /* Fill render */
    SDL_SetRenderDrawColor(w->ren,
                           (w->off >> 16) & 0xFF,
                           (w->off >>  8) & 0xFF,
                           (w->off >>  0) & 0xFF,
                           0xFF);
    SDL_RenderClear(w->ren);

    /* on pixels */
    SDL_SetRenderDrawColor(w->ren,
                           (w->on  >> 16) & 0xFF,
                           (w->on  >>  8) & 0xFF,
                           (w->on  >>  0) & 0xFF,
                           0xFF);

    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 64; ++x) {
            if (c->FB[y * 64 + x]) {
                r.x = x * scale;
                r.y = y * scale;
                SDL_RenderFillRect(w->ren, &r);
            }
        }
    }
    SDL_RenderPresent(w->ren);
}


void sdl_palette(window_t *w, int idx)
{
    idx &= 1;
    w->on  = palettes[idx][0];
    w->off = palettes[idx][1];
}
