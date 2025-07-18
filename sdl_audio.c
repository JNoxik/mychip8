#include "sdl.h"
#include <math.h>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static SDL_AudioDeviceID audio_dev = 0;
static SDL_AudioStream  *audio_stream = NULL;
int                      audio_volume = 30;

static const int FREQ   = 48000;
static const int TARGET_MS = 200;
static const int SAMPLES_PER_FRAME = FREQ / 60;



void sdl_audio_init(void)
{
    SDL_AudioSpec want = {0};
    want.freq   = FREQ;
    want.format = SDL_AUDIO_S16;
    want.channels = 1;

    audio_dev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &want);
    if (audio_dev) {
        audio_stream = SDL_CreateAudioStream(&want, &want);
        SDL_BindAudioStream(audio_dev, audio_stream);
    }
}

void sdl_audio_destroy(void)
{
    if (audio_stream) SDL_DestroyAudioStream(audio_stream);
    if (audio_dev)    SDL_CloseAudioDevice(audio_dev);
    audio_stream = NULL;
    audio_dev = 0;
}


static void generate_square(int16_t *dst, int count)
{
    static double phase = 0.0;
    const double step = 440.0 / FREQ * 2.0 * M_PI;
    int16_t level = (int16_t)(2500 * audio_volume / 100.0);

    for (int i = 0; i < count; ++i) {
        dst[i] = (sin(phase) >= 0.0) ? level : -level; /* volume */
        phase += step;
        if (phase >= 2.0 * M_PI) phase -= 2.0 * M_PI;
    }
}

void sdl_audio_sound(bool active)
{
    if (!audio_dev || !audio_stream) return;

    Uint64 queued_bytes = SDL_GetAudioStreamQueued(audio_stream);
    Uint64 queued_ms    = queued_bytes * 1000 / 48000;

    if (queued_ms < (Uint64)TARGET_MS) {
        int16_t buf[SAMPLES_PER_FRAME];
        generate_square(buf, SAMPLES_PER_FRAME);
        SDL_PutAudioStreamData(audio_stream, buf, sizeof(buf));
    }

    if (active)
        SDL_ResumeAudioDevice(audio_dev);
    else
        SDL_PauseAudioDevice(audio_dev);
}
