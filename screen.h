#ifndef SCREEN_H
#define SCREEN_H

#include <string.h>

#include "src/gpu.h"
#include "src/uart.h"

#include "SDL2/SDL.h"

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;

    psx_gpu_t* gpu;
    uart_t* uart;

    unsigned int saved_scale;
    unsigned int width, height, scale;
    unsigned int format;

    int open, debug_mode;
} screen_t;

screen_t* screen_create();
void screen_init(screen_t*, psx_gpu_t*, uart_t*);
void screen_reload(screen_t*);
int screen_is_open(screen_t*);
void screen_update(screen_t*);
void screen_destroy(screen_t*);
void screen_set_scale(screen_t*, unsigned int);
void screen_toggle_debug_mode(screen_t*);

// GPU event handlers
void gpu_dmode_event_cb(psx_gpu_t*);
void gpu_vblank_event_cb(psx_gpu_t*);

#endif