#include <SDL3/SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "defines.h"
#include "vector.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360

#define TEXTURE_WIDTH SCREEN_WIDTH
#define TEXTURE_HEIGHT SCREEN_HEIGHT

u32 pixel_buffer[TEXTURE_WIDTH * TEXTURE_HEIGHT];

void draw_point(Vector2 point) {
    u32 x = (u32)floor(point.x);
    u32 y = (u32)floor(point.y);

    if ((x >= TEXTURE_WIDTH) || (y >= TEXTURE_HEIGHT)) {
        return;
    }

    pixel_buffer[y * TEXTURE_WIDTH + x] = 0xFFFFFFFF;
}

// https://haqr.eu/tinyrenderer/bresenham/
// by Dmitry V. Sokolov under zlib licence
void draw_line(Vector2 a, Vector2 b) {
    b32 steep = fabs(a.x - b.x) < fabs(a.y - b.y);
    if (steep) {
        SWAP(f32, a.x, a.y);
        SWAP(f32, b.x, b.y);
    }
    if (a.x > b.x) {
        SWAP(Vector2, a, b);
    }
    i32 y = a.y;
    i32 ierror = 0;
    for (i32 x = a.x; x <= b.x; x++) {
        if (steep) {
            draw_point((Vector2){y, x});
        } else {
            draw_point((Vector2){x, y});
        }
        ierror += 2 * fabs(b.y - a.y);
        y += (b.y > a.y ? 1 : -1) * (ierror > b.x - a.x);
        ierror -= 2 * (b.x - a.x) * (ierror > b.x - a.x);
    }
}

typedef struct KLine {
    Vector2 a;
    Vector2 b;
    b32 dead;
} KLine;

#define MAX_LINES 5000

void push_line(KLine* lines, KLine line, i32* count) {
    if (*count < MAX_LINES) {
        lines[*count] = line;
        *count = *count + 1;
    }
    return;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Could not initalize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window* window =
        SDL_CreateWindow("Koch", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
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

    b32 app_running = TRUE;

    SDL_Event event;

    KLine* lines = (KLine*)malloc(sizeof(KLine) * MAX_LINES);
    i32 line_count = 0;
    push_line(lines,
              (KLine){(Vector2){0, TEXTURE_HEIGHT},
                      (Vector2){TEXTURE_WIDTH, TEXTURE_HEIGHT}, 0},
              &line_count);
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
        i32 current_lines = MIN(line_count, MAX_LINES);
        for (i32 j = 0; (j < current_lines) && (line_count < MAX_LINES); j++) {
            if (!lines[j].dead) {
                KLine current_line = lines[j];
                lines[j].dead = TRUE;

                Vector2 p1 =
                    vector2_lerp(current_line.a, current_line.b, 1.0 / 3.0);
                Vector2 p2 =
                    vector2_lerp(current_line.a, current_line.b, 2.0 / 3.0);

                KLine s1 = {current_line.a, p1, 0};
                KLine s3 = {p2, current_line.b, 0};

                push_line(lines, s1, &line_count);
                push_line(lines, s3, &line_count);

                Vector2 midpoint = vector2_lerp(p1, p2, 1.0 / 2.0);
                Vector2 orth =
                    vector2_normalize(vector2_orthogonal(vector2_sub(p1, p2)));

                // Height of an equilateral triangle
                f32 height =
                    sqrt(3.0) / 2.0 * vector2_magnitude(vector2_sub(p1, p2));

                // The tip of the new triangle
                Vector2 p3 = vector2_add(vector2_scale(orth, height), midpoint);

                KLine s2a = {p1, p3, 0};
                KLine s2b = {p3, p2, 0};

                push_line(lines, s2a, &line_count);
                push_line(lines, s2b, &line_count);
            }
        }

        // render
        memset(pixel_buffer, 0, sizeof(pixel_buffer));

        for (i32 k = 0; k < MIN(line_count, MAX_LINES); k++) {
            if (lines[k].dead) {
                continue;
            }
            draw_line(lines[k].a, lines[k].b);
        }

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

        SDL_RenderPresent(renderer);
    }

    free(lines);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
