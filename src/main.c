#include <SDL3/SDL.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define u8 uint8_t
#define u32 uint32_t
#define u64 uint64_t

#define b8 int8_t
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

#define INIT_SCALE_FACTOR 200

#define OFFSET_X -2
#define OFFSET_Y -1

// TODO(novel): Make this controllable via a slider in the ui
#define MAX_ITERATIONS 100

#define VERY_SMALL 0.0001

typedef struct Vector2 {
    f32 x;
    f32 y;
} Vector2;

f32 vector2_magnitude(Vector2 v) { return sqrt(v.x * v.x + v.y * v.y); }
f32 vector2_mag_sqared(Vector2 v) { return v.x * v.x + v.y * v.y; }

// zero vector stays zero
Vector2 vector2_normalize(Vector2 v) {
    if (vector2_mag_sqared(v) < VERY_SMALL) {
        return v;
    }
    Vector2 result = {
        v.x / vector2_magnitude(v),
        v.y / vector2_magnitude(v),
    };
    return result;
}

Vector2 vector2_add(Vector2 a, Vector2 b) {
    Vector2 result = {a.x + b.x, a.y + b.y};
    return result;
}

Vector2 vector2_sub(Vector2 a, Vector2 b) {
    Vector2 result = {a.x - b.x, a.y - b.y};
    return result;
}

Vector2 vector2_scale(Vector2 v, f32 s) {
    Vector2 result = {v.x * s, v.y * s};
    return result;
}

Vector2 screen_to_world(Vector2 screen_pos, Vector2 camera_pos,
                        f32 camera_zoom) {
    Vector2 offset = {
        -SCREEN_WIDTH / 2.0,
        -SCREEN_HEIGHT / 2.0,
    };
    Vector2 result = vector2_add(
        vector2_scale(vector2_add(screen_pos, offset), 1 / camera_zoom),
        camera_pos);
    return result;
}

int main(int argc, char** argv) {
    (void) argc;
    (void) argv;

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

    f64 last_frame_time = 0.0;
    char frame_time_text_buffer[32];
    char fps_text_buffer[32];

    Vector2 camera_pos = {0.0, 0.0};
    Vector2 camera_velocity = {0.0, 0.0};
    f32 camera_max_speed = 0.01;
    f32 camera_zoom = INIT_SCALE_FACTOR;
    f32 camera_zoom_sensitivity = 0.0;
    f32 scroll_wheel = 0.0f;

    b8 mouse_dragging = FALSE;
    Vector2 mouse_pos = {0.0, 0.0};
    Vector2 last_mouse_pos = {0.0, 0.0};

    // TODO(novel): Add delta time
    while (app_running) {
        // timing
        u64 frame_start = SDL_GetPerformanceCounter();

        // poll
        scroll_wheel = 0.0f;
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
            case SDL_EVENT_MOUSE_WHEEL: {
                scroll_wheel = event.wheel.y;
            } break;

            default:
                break;
            }
        }

        last_mouse_pos = mouse_pos;
        mouse_dragging =
            SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y) & SDL_BUTTON_LMASK;

        if (mouse_dragging) {
            Vector2 world_pos =
                screen_to_world(mouse_pos, camera_pos, camera_zoom);
            Vector2 last_world_pos =
                screen_to_world(last_mouse_pos, camera_pos, camera_zoom);

            printf("X: %.2f, Y: %.2f\n", world_pos.x, world_pos.y);
            fflush(stdout);

            Vector2 mouse_world_diff = vector2_sub(last_world_pos, world_pos);
            camera_pos = vector2_add(camera_pos, mouse_world_diff);
        }


        camera_zoom_sensitivity = fabs(camera_zoom) / 10;
        if (scroll_wheel > VERY_SMALL) {
            camera_zoom += camera_zoom_sensitivity;
            printf("Zoom: %.2f\n", camera_zoom);
            fflush(stdout);
        } else if (scroll_wheel < -VERY_SMALL) {
            camera_zoom -= camera_zoom_sensitivity;
            printf("Zoom: %.2f\n", camera_zoom);
            fflush(stdout);
        }

        camera_max_speed = 1 / camera_zoom_sensitivity;

        b8* keyboard = (b8*)SDL_GetKeyboardState(NULL);
        camera_velocity.x = 0.0;
        camera_velocity.y = 0.0;
        if (keyboard[SDL_SCANCODE_W]) {
            camera_velocity.y -= 1;
        }
        if (keyboard[SDL_SCANCODE_S]) {
            camera_velocity.y += 1;
        }
        if (keyboard[SDL_SCANCODE_A]) {
            camera_velocity.x -= 1;
        }
        if (keyboard[SDL_SCANCODE_D]) {
            camera_velocity.x += 1;
        }

        // should stay 0 when input is 0
        camera_velocity = vector2_normalize(camera_velocity);
        camera_velocity = vector2_scale(camera_velocity, camera_max_speed);

        // update

        camera_pos.x += camera_velocity.x;
        camera_pos.y += camera_velocity.y;

        snprintf(frame_time_text_buffer, sizeof(frame_time_text_buffer),
                 "Frame Time: %.2f ms", last_frame_time * 1000.0);
        snprintf(fps_text_buffer, sizeof(fps_text_buffer), "FPS: %d",
                 (int)(1.0 / last_frame_time));

        for (int y = 0; y < TEXTURE_HEIGHT; y++) {
            for (int x = 0; x < TEXTURE_WIDTH; x++) {
                Vector2 p0 =
                    screen_to_world((Vector2){x, y}, camera_pos, camera_zoom);
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

        SDL_SetRenderDrawColor(renderer, 20, 100, 60, 255);
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
