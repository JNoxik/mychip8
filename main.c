/* main.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chip8.h"
#include "dbg.h"
#include "sdl.h"

typedef struct {
    const char *rom_path;
    int         palette_idx;   /* 0 – bw, 1 – amber */
    int         scale;
    int         hz;
    int         volume;
    bool        nosound;
    int         debug;
} cfg_t;


static void usage(const char *prog)
{
    fprintf(stderr,
            "Usage: %s -f <*.ch8 / *.rom> [-p bw|amber]\n"
            "  -f   ROM-file\n"
            "  -p   palette (bw or amber, default bw)\n"
            "  -s   pixel scale (default 20)\n"
            "  -hz  CPU frequency (default 500)\n"
            "  -nosound  Disable sound\n"
            "  -debug    Debug mode (0 - disable,\n"
            "                        1 - log to file,\n"
            "                        2 - step-by-step)\n"
            , prog);
}

static cfg_t parse_args(int argc, char *argv[])
{
    /* by default */
    cfg_t cfg = {0};
    cfg.palette_idx = 0;
    cfg.scale = 20;
    cfg.hz = 500;
    cfg.volume = 30;
    cfg.nosound = false;
    cfg.debug = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            cfg.rom_path = argv[++i];
        } 
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            const char *p = argv[++i];
            if      (!strcmp(p, "amber")) cfg.palette_idx = 1;
            else if (!strcmp(p, "bw"))    cfg.palette_idx = 0;
            else { usage(argv[0]); exit(EXIT_FAILURE); }
        } 
        else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            cfg.scale = atoi(argv[++i]);
            if (cfg.scale < 1) {
                fprintf(stderr, "Scale must be >1\n");
                exit(EXIT_FAILURE);
            }
        } 
        else if (strcmp(argv[i], "-hz") == 0 && i + 1 < argc) {
            cfg.hz = atoi(argv[++i]);
            if (cfg.hz < 100) {
                fprintf(stderr, "Hz must be >100\n");
                exit(EXIT_FAILURE);
            }
        } 
        else if (strcmp(argv[i], "-v") == 0 && i + 1 < argc) {
            cfg.volume = atoi(argv[++i]);
            if (cfg.volume < 0 || cfg.volume > 100) {
                fprintf(stderr, "Volume must be 0…100\n");
                exit(EXIT_FAILURE);
            }
        } 
        else if (strcmp(argv[i], "-nosound") == 0) {
            cfg.nosound = true;
        }
        else if (strcmp(argv[i], "-debug") == 0 && i + 1 < argc) {
            cfg.debug = atoi(argv[++i]);
            if (cfg.debug < 0 || cfg.debug > 2) cfg.debug = 0;
        }
        else {
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (!cfg.rom_path) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    return cfg;
}


static bool load_rom(chip8_t *c, const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return false; }

    size_t bytes = fread(c->memory.memory + 0x200, 1, MEM_SIZE - 0x200, f);
    fclose(f);

    if (bytes == 0) { fprintf(stderr, "Empty ROM\n"); return false; }
    return true;
}


static void handle_events(chip8_t *c, window_t *w, bool *running)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_EVENT_QUIT) {
            *running = false;
        } else if (ev.type == SDL_EVENT_KEY_DOWN) {
            if (ev.key.key == SDLK_F1) sdl_palette(w, 0);
            else if (ev.key.key == SDLK_F2) sdl_palette(w, 1);
            else {
                for (int i = 0; i < 16; ++i) {
                    if (ev.key.key == keymap[i]) {
                        c->keypad[i] = 1;
                        break;
                    }
                }
            }
        } else if (ev.type == SDL_EVENT_KEY_UP) {
            for (int i = 0; i < 16; ++i) {
                if (ev.key.key == keymap[i]) {
                    c->keypad[i] = 0;
                    break;
                }
            }
        }
    }
}


int main(int argc, char *argv[])
{
    cfg_t cfg = parse_args(argc, argv);

    chip8_t *chip8 = chip8_init();
    if (!chip8 || !load_rom(chip8, cfg.rom_path)) {
        fprintf(stderr, "Cannot load ROM\n");
        return EXIT_FAILURE;
    }
    chip8->PC = 0x200;

    debug_init(cfg.debug);

    window_t *win = sdl_init(cfg.scale);
    if (!win) {
        chip8_destroy(chip8);
        return EXIT_FAILURE;
    }
    sdl_palette(win, cfg.palette_idx);


    if (!cfg.nosound) {
        audio_volume = cfg.volume;
        sdl_audio_init();
    }

    bool running = true;
    uint64_t last_cycle = SDL_GetTicks();
    uint64_t last_timer = last_cycle;
    uint64_t cycles_accum = 0;

    while (running) {
        handle_events(chip8, win, &running);

        if (!cfg.nosound)
            sdl_audio_sound(chip8->ST > 0);

        uint64_t now = SDL_GetTicks();
        uint64_t delta = now - last_cycle;
        last_cycle = now;

        cycles_accum += delta * cfg.hz / 1000;
        while (cycles_accum > 0) {
            chip8_cycle(chip8);
            if (cfg.debug == 2) break;
            cycles_accum--;
        }

        if (now - last_timer >= 1000 / 60) {
            chip8_update(chip8);
            sdl_audio_sound(chip8->ST > 0);
            last_timer = now;
        }

        sdl_draw(chip8, win, cfg.scale);
    }

    sdl_audio_destroy();
    sdl_destroy(win);
    chip8_destroy(chip8);
    debug_destroy();
    return 0;
}
