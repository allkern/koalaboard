#include "screen.h"

#include <stdint.h>

uint32_t screen_get_dmode_width(screen_t* screen) {
    static int dmode_hres_table[] = {
        256, 320, 512, 640
    };

    if (screen->gpu->display_mode & 0x40) {
        return 368;
    } else {
        return dmode_hres_table[screen->gpu->display_mode & 0x3];
    }
}

uint32_t screen_get_dmode_height(screen_t* screen) {
    return (screen->gpu->display_mode & 0x4) ? 480 : 240;
}

uint32_t screen_get_display_format(screen_t* screen) {
    return (screen->gpu->display_mode >> 4) & 1;
}

double psx_get_display_aspect(screen_t* screen) {
    double width = screen_get_dmode_width(screen);
    double height = screen_get_dmode_height(screen);

    if (height > width)
        return 4.0 / 3.0;

    double aspect = width / height;

    if (aspect > (4.0 / 3.0))
        return 4.0 / 3.0;

    return aspect;
}

int screen_get_base_width(screen_t* screen) {
    int width = screen_get_dmode_width(screen);

    return (width == 256) ? 256 : 320;
}

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

    screen->texture_width = PSX_GPU_FB_WIDTH;
    screen->texture_height = PSX_GPU_FB_HEIGHT;
    screen->bilinear = 1;

    SDL_SetRenderDrawColor(screen->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
}

void screen_reload(screen_t* screen) {
    if (screen->texture) SDL_DestroyTexture(screen->texture);
    if (screen->renderer) SDL_DestroyRenderer(screen->renderer);
    if (screen->window) SDL_DestroyWindow(screen->window);

    if (screen->debug_mode) {
        screen->width = PSX_GPU_FB_WIDTH;
        screen->height = PSX_GPU_FB_HEIGHT;
    } else {
        if (screen->vertical_mode) {
            screen->width = 240;
            screen->height = screen_get_base_width(screen);
        } else {
            screen->width = screen_get_base_width(screen);
            screen->height = 240;
        }
    }

    screen->window = SDL_CreateWindow(
        "koalaboard",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        screen->width * screen->scale,
        screen->height * screen->scale,
        0
    );

    screen->renderer = SDL_CreateRenderer(
        screen->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "linear"),

    screen->texture = SDL_CreateTexture(
        screen->renderer,
        screen->format,
        SDL_TEXTUREACCESS_STREAMING,
        screen->texture_width, screen->texture_height
    );

    SDL_SetTextureScaleMode(screen->texture, screen->bilinear);

    // Check for retina displays
    int width = 0, height = 0;

    SDL_GetRendererOutputSize(screen->renderer, &width, &height);

    if (width != (screen->width * screen->scale)) {
        float width_scale = (float)width / (float)(screen->width * screen->scale);
        float height_scale = (float)height / (float)(screen->height * screen->scale);

        SDL_RenderSetScale(screen->renderer, width_scale, height_scale);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);

    screen->open = 1;
}

int screen_is_open(screen_t* screen) {
    return screen->open;
}

void screen_toggle_debug_mode(screen_t* screen) {
    screen->debug_mode = !screen->debug_mode;

    screen_set_scale(screen, screen->saved_scale);

    screen->texture_width = PSX_GPU_FB_WIDTH;
    screen->texture_height = PSX_GPU_FB_HEIGHT;

    gpu_dmode_event_cb(screen->gpu);
}

void screen_update(screen_t* screen) {
    // void* vram = psx_get_vram(screen->psx);

    // if (screen->field & 2) {
    //     for (int y = screen->field & 1; y < 512; y += 2) {
    //         memcpy(
    //             ((uint8_t*)screen->buf) + (y * PSX_GPU_FB_STRIDE),
    //             ((uint8_t*)vram) + (y * PSX_GPU_FB_STRIDE),
    //             PSX_GPU_FB_STRIDE
    //         );
    //     }
    // }

    // screen->field += 1;
    // screen->field &= 3;

    void* display_buf = screen->debug_mode ?
        screen->gpu->vram : psx_gpu_get_display_buffer(screen->gpu);

    SDL_UpdateTexture(screen->texture, NULL, display_buf, PSX_GPU_FB_STRIDE);
    SDL_RenderClear(screen->renderer);

    if (!screen->debug_mode) {
        SDL_Rect dstrect;

        dstrect.x = screen->image_xoff;
        dstrect.y = screen->image_yoff;
        dstrect.w = screen->image_width;
        dstrect.h = screen->image_height;

        SDL_RenderCopyEx(
            screen->renderer,
            screen->texture,
            NULL, &dstrect,
            screen->vertical_mode ? 270 : 0,
            NULL, SDL_FLIP_NONE
        );
    } else {
        SDL_RenderCopy(screen->renderer, screen->texture, NULL, NULL);
    }

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
                            screen->width,
                            screen->height,
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

                    case SDLK_F3: {
                        screen->vertical_mode = !screen->vertical_mode;

                        gpu_dmode_event_cb(screen->gpu);
                    } break;

                    case SDLK_F4: {
                        screen->bilinear = !screen->bilinear;

                        gpu_dmode_event_cb(screen->gpu);
                    } break;

                    case SDLK_F11: {
                        screen->fullscreen = !screen->fullscreen;

                        SDL_SetWindowFullscreen(
                            screen->window,
                            screen->fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0
                        );

                        gpu_dmode_event_cb(screen->gpu);

                        SDL_SetRenderDrawColor(screen->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                        SDL_RenderClear(screen->renderer);
                    } break;

                    // case SDLK_F5: {
                    //     psx_soft_reset(screen->psx);
                    // } break;

                    // case SDLK_F6: {
                    //     psx_swap_disc(screen->psx, ".\\roms\\Street Fighter II Movie (Japan) (Disc 2)\\Street Fighter II Movie (Japan) (Disc 2).cue");
                    // } break;
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

    screen->format = screen_get_display_format(screen) ?
        SDL_PIXELFORMAT_RGB24 : SDL_PIXELFORMAT_BGR555;

    if (screen->debug_mode) {
        screen->width = PSX_GPU_FB_WIDTH;
        screen->height = PSX_GPU_FB_HEIGHT;
        screen->texture_width = PSX_GPU_FB_WIDTH;
        screen->texture_height = PSX_GPU_FB_HEIGHT;
    } else {
        if (screen->fullscreen) {
            SDL_DisplayMode dm;
            SDL_GetCurrentDisplayMode(0, &dm);

            screen->width = dm.w;
            screen->height = dm.h;

            if (screen->vertical_mode) {
                screen->image_width = screen->height;
                screen->image_height = (double)screen->height / psx_get_display_aspect(screen);

                int off = (screen->image_width - screen->image_height) / 2;

                screen->image_xoff = (screen->width / 2) - (screen->image_width / 2);
                screen->image_yoff = off;
            } else {
                screen->image_width = (double)screen->height * psx_get_display_aspect(screen);
                screen->image_height = screen->height;
                screen->image_xoff = (screen->width / 2) - (screen->image_width / 2);
                screen->image_yoff = 0;
            }
        } else {
            if (screen->vertical_mode) {
                screen->width = 240 * screen->scale;
                screen->height = screen_get_base_width(screen) * screen->scale;
                screen->image_width = screen->height;
                screen->image_height = screen->width;

                int off = (screen->image_width - screen->image_height) / 2;

                screen->image_xoff = -off;
                screen->image_yoff = off;
            } else {
                screen->width = screen_get_base_width(screen) * screen->scale;
                screen->height = 240 * screen->scale;
                screen->image_width = screen->width;
                screen->image_height = screen->height;
                screen->image_xoff = 0;
                screen->image_yoff = 0;
            }
        }

        screen->texture_width = screen_get_dmode_width(screen);
        screen->texture_height = screen_get_dmode_height(screen);
    }

    SDL_DestroyTexture(screen->texture);

    screen->texture = SDL_CreateTexture(
        screen->renderer,
        screen->format,
        SDL_TEXTUREACCESS_STREAMING,
        screen->texture_width, screen->texture_height
    );

    SDL_SetTextureScaleMode(screen->texture, screen->bilinear);

    SDL_SetWindowSize(screen->window, screen->width, screen->height);
}

void gpu_vblank_event_cb(psx_gpu_t* gpu) {
    screen_t* screen = gpu->udata[0];

    screen_update(screen);
}