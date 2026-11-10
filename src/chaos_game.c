#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "defines.h"
#include "vector.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360

#define TEXTURE_WIDTH SCREEN_WIDTH
#define TEXTURE_HEIGHT SCREEN_HEIGHT

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Could not initalize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window* window =
        SDL_CreateWindow("Chaos Game", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
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

    srand(time(NULL));

    // https://en.wikipedia.org/wiki/Sierpiński_triangle#Chaos_game
    // I will be pasting in the steps verbatim

    // 1. Take three points in a plane to form a triangle.
    Vector2 A = {SCREEN_WIDTH / 2, 0};
    Vector2 B = {SCREEN_WIDTH, SCREEN_HEIGHT};
    Vector2 C = {0, SCREEN_HEIGHT};

    Vector2 Verts[3] = {A, B, C};

    // // https://en.wikipedia.org/wiki/Barycentric_coordinate_system

    // f32 wA = rand() / (f32)RAND_MAX;
    // f32 wB = rand() / (f32)RAND_MAX;
    // f32 wC = rand() / (f32)RAND_MAX;
    //
    // f32 weight_sum = wA + wB + wC;
    // wA /= weight_sum;
    // wB /= weight_sum;
    // wC /= weight_sum;

    // "Generating Random Points in Triangles"
    // by Greg Turk from Graphics Gems (1990)

    // Also there was like zero need to do this, but its here for reference now.

    // 2. Randomly select any point inside the triangle and consider that your
    // current position.
    f32 s = rand() / (f32)RAND_MAX;
    f32 t = rand() / (f32)RAND_MAX;
    f32 rt = sqrtf(t);

    f32 wA = 1 - rt;
    f32 wB = (1 - s) * rt;
    f32 wC = s * rt;

    Vector2 P = {A.x * wA + B.x * wB + C.x * wC,
                 A.y * wA + B.y * wB + C.y * wC};

    while (app_running) {
        // poll
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                app_running = FALSE;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_Q) {
                    app_running = FALSE;
                }
                break;
            default:
                break;
            }
        }

        // update

        // 3. Randomly select any one of the three vertex points.
        Vector2 V = Verts[rand() % 3];

        // 4. Move half the distance from your current position
        // to the selected vertex.
        //Vector2 last_p = P;
        P = (Vector2){(P.x + V.x) / 2.0, (P.y + V.y) / 2.0};

        // 5. Plot the current position.

        // render
        u8 r = 255;
        u8 g = 255;
        u8 b = 255;

        pixel_buffer[(i32)floor(P.y) * TEXTURE_WIDTH + (i32)floor(P.x)] =
            0xFF000000 | ((u32)r << 16) | ((u32)g << 8) | b;
        //
        // for (int y = 0; y < TEXTURE_HEIGHT; y++) {
        //     for (int x = 0; x < TEXTURE_WIDTH; x++) {
        //
        //         u8 r = 255;
        //         u8 g = 255;
        //         u8 b = 255;
        //
        //         pixel_buffer[y * TEXTURE_WIDTH + x] =
        //             0xFF000000 | ((u32)r << 16) | ((u32)g << 8) | b;
        //     }
        // }

        void* texture_pixels;
        int pitch;
        // TODO(novel): Draw the pixels directly in here
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

        //SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        //SDL_RenderLine(renderer, last_p.x, last_p.y, P.x, P.y);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
