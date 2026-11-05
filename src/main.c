#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define u8 uint8_t
#define u32 uint32_t
#define u64 uint64_t

#define b32 int32_t

#define f32 float
#define f64 double

#define TRUE 1
#define FALSE 0

#define TARGET_FPS 30
#define TARGET_FRAME_TIME (1.0 / TARGET_FPS)

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360

#define TEXTURE_WIDTH SCREEN_WIDTH
#define TEXTURE_HEIGHT SCREEN_HEIGHT

#define SCALE_FACTOR 200

#define OFFSET_X -2
#define OFFSET_Y -1

#define MAX_ITERATIONS 40

typedef struct Vector2 {
    f32 x;
    f32 y;
} Vector2;

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Could not initalize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window* window =
        SDL_CreateWindow("Mandelbrot", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (window == NULL) {
        printf("Could not create window: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL) {
        printf("Could not create renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             TEXTURE_WIDTH, TEXTURE_HEIGHT);

    u32 pixel_buffer[TEXTURE_WIDTH * TEXTURE_HEIGHT];

    b32 app_running = TRUE;

    SDL_Event event;

    f32 last_frame_time = 0.0;
    char frame_time_text_buffer[32];
    char fps_text_buffer[32];

    while (app_running) {
        // timing
        u64 frame_start = SDL_GetPerformanceCounter();

        // poll
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                app_running = FALSE;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    app_running = FALSE;
                }
                break;
            default:
                break;
            }
        }

        // update
        snprintf(frame_time_text_buffer, sizeof(frame_time_text_buffer),
                 "Frame Time: %.2f ms", last_frame_time * 1000.0);
        snprintf(fps_text_buffer, sizeof(fps_text_buffer), "FPS: %d",
                 (int)(1.0 / last_frame_time));

        for (int y = 0; y < TEXTURE_HEIGHT; y++) {
            for (int x = 0; x < TEXTURE_WIDTH; x++) {

                Vector2 p0 = {x / (f32)SCALE_FACTOR + OFFSET_X,
                              y / (f32)SCALE_FACTOR + OFFSET_Y};
                Vector2 p = {0.0, 0.0};
                Vector2 p2 = {0.0, 0.0};

                int iteration = 0;

                // https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Optimized_escape_time_algorithms
                while ((p2.x + p2.y <= 4) && (iteration < MAX_ITERATIONS)) {
                    p2.x = p.x * p.x;
                    p2.y = p.y * p.y;
                    p.y = 2 * p.x * p.y + p0.y;
                    p.x = p2.x - p2.y + p0.x;
                    iteration++;
                }

                // TODO(novel): Color
                u8 r = (u8)(iteration / MAX_ITERATIONS * 255);
                u8 g = r;
                u8 b = r;

                pixel_buffer[y * TEXTURE_WIDTH + x] =
                    0xFF000000 | ((u32)r << 16) | ((u32)g << 8) | b;
            }
        }

        // render
        void* texture_pixels;
        int pitch;
        if (SDL_LockTexture(texture, NULL, &texture_pixels, &pitch)) {
            for (int y = 0; y < TEXTURE_HEIGHT; y++) {
                memcpy((u8*)texture_pixels + y * pitch,
                       &pixel_buffer[y * TEXTURE_WIDTH],
                       TEXTURE_WIDTH * sizeof(u32));
            }
            SDL_UnlockTexture(texture);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderDebugText(renderer, 10.0f, 10.0f, frame_time_text_buffer);
        SDL_RenderDebugText(renderer, 10.0f, 20.0f, fps_text_buffer);

        SDL_RenderPresent(renderer);

        // timing
        u64 frame_end = SDL_GetPerformanceCounter();

        f64 elapsed =
            (f64)(frame_end - frame_start) / SDL_GetPerformanceFrequency();
        f64 remaining = TARGET_FRAME_TIME - elapsed;
        if (remaining > 0.0) {
            SDL_DelayNS((u64)(remaining * 1e9));
        }

        last_frame_time = (f64)(SDL_GetPerformanceCounter() - frame_start) /
                          SDL_GetPerformanceFrequency();
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
