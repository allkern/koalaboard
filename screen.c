#include "screen.h"

screen_t* screen_create() {
    return (screen_t*)malloc(sizeof(screen_t));
}

void screen_init(screen_t* screen, psx_gpu_t* gpu, uart_t* uart) {
    memset(screen, 0, sizeof(screen_t));

    if (screen->debug_mode) {
        screen->width = PSX_GPU_FB_WIDTH;
        screen->height = PSX_GPU_FB_HEIGHT;
    } else {
        screen->width = 320;
        screen->height = 240;
    }

    screen->scale = 1;
    screen->open = 1;
    screen->format = SDL_PIXELFORMAT_BGR555;
    screen->gpu = gpu;
    screen->uart = uart;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "0");
}

void screen_reload(screen_t* screen) {
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "0");

    if (screen->texture) SDL_DestroyTexture(screen->texture);
    if (screen->renderer) SDL_DestroyRenderer(screen->renderer);
    if (screen->window) SDL_DestroyWindow(screen->window);

    if (screen->debug_mode) {
        screen->width = PSX_GPU_FB_WIDTH;
        screen->height = PSX_GPU_FB_HEIGHT;
    } else {
        screen->width = 320;
        screen->height = 240;
    }

    screen->window = SDL_CreateWindow(
        "machine",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        screen->width * screen->scale,
        screen->height * screen->scale,
        SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
    );

    screen->renderer = SDL_CreateRenderer(
        screen->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    screen->texture = SDL_CreateTexture(
        screen->renderer,
        screen->format,
        SDL_TEXTUREACCESS_STREAMING,
        PSX_GPU_FB_WIDTH, PSX_GPU_FB_HEIGHT
    );

    // Check for retina displays
    int width = 0, height = 0;

    SDL_GetRendererOutputSize(screen->renderer, &width, &height);

    if (width != (screen->width * screen->scale)) {
        float width_scale = (float)width / (float)(screen->width * screen->scale);
        float height_scale = (float)height / (float)(screen->height * screen->scale);

        SDL_RenderSetScale(screen->renderer, width_scale, height_scale);
    }

    screen->open = 1;
}

int screen_is_open(screen_t* screen) {
    return screen->open;
}

void screen_toggle_debug_mode(screen_t* screen) {
    screen->debug_mode = !screen->debug_mode;

    screen_set_scale(screen, screen->saved_scale);

    gpu_dmode_event_cb(screen->gpu);
}

void screen_update(screen_t* screen) {
    void* display_buf = screen->debug_mode ?
        screen->gpu->vram : psx_gpu_get_display_buffer(screen->gpu);

    SDL_UpdateTexture(screen->texture, NULL, display_buf, PSX_GPU_FB_STRIDE);
    SDL_RenderCopy(screen->renderer, screen->texture, NULL, NULL);
    SDL_RenderPresent(screen->renderer);

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                screen->open = 0;
            } break;

            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: {
                        uart_send_byte(screen->uart, 0x1b);
                    } break;

                    case SDLK_RETURN: {
                        uart_send_byte(screen->uart, '\r');
                    } break;

                    case SDLK_BACKSPACE: {
                        uart_send_byte(screen->uart, '\b');
                    } break;

                    case SDLK_F1: {
                        screen_toggle_debug_mode(screen);

                        return;
                    } break;

                    case SDLK_F2: {
                        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
                            0,
                            screen->width * screen->scale,
                            screen->height * screen->scale,
                            16,
                            SDL_PIXELFORMAT_BGR555
                        );

                        SDL_RenderReadPixels(
                            screen->renderer,
                            NULL,
                            SDL_PIXELFORMAT_BGR555,
                            surface->pixels, surface->pitch
                        );

                        SDL_SaveBMP(surface, "snap/screenshot.bmp");

                        SDL_FreeSurface(surface);
                    } break;
                }
            } break;

            case SDL_TEXTINPUT: {
                uart_send_byte(screen->uart, event.text.text[0]);
            } break;
        }
    }
}

void screen_set_scale(screen_t* screen, unsigned int scale) {
    if (screen->debug_mode) {
        screen->scale = 1;
    } else {
        screen->scale = scale;
        screen->saved_scale = scale;
    }
}

void screen_destroy(screen_t* screen) {
    SDL_DestroyTexture(screen->texture);
    SDL_DestroyRenderer(screen->renderer);
    SDL_DestroyWindow(screen->window);

    SDL_Quit();

    free(screen);
}

void gpu_dmode_event_cb(psx_gpu_t* gpu) {
    screen_t* screen = gpu->udata[0];

    int texture_width;
    int texture_height;

    screen->format = (screen->gpu->display_mode & 0x10) ?
        SDL_PIXELFORMAT_BGR555 : SDL_PIXELFORMAT_RGB888;

    if (screen->debug_mode) {
        screen->width = PSX_GPU_FB_WIDTH;
        screen->height = PSX_GPU_FB_HEIGHT;
        texture_width = PSX_GPU_FB_WIDTH;
        texture_height = PSX_GPU_FB_HEIGHT;
    } else {
        screen->width = 320;
        screen->height = 240;

        static int dmode_hres_table[] = {
            256, 320, 512, 640
        };

        if (screen->gpu->display_mode & 0x40) {
            texture_width = 368;
        } else {
            texture_width = dmode_hres_table[screen->gpu->display_mode & 0x3];
        }

        texture_height = (screen->gpu->display_mode & 0x4) ? 480 : 240;
    }

    SDL_DestroyTexture(screen->texture);

    screen->texture = SDL_CreateTexture(
        screen->renderer,
        SDL_PIXELFORMAT_BGR555,
        SDL_TEXTUREACCESS_STREAMING,
        texture_width, texture_height
    );

    SDL_SetWindowSize(
        screen->window,
        screen->width * screen->scale,
        screen->height * screen->scale
    );
}

void gpu_vblank_event_cb(psx_gpu_t* gpu) {
    screen_t* screen = gpu->udata[0];

    screen_update(screen);
}