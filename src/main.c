#include <SDL3/SDL.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define u8 uint8_t
#define u32 uint32_t
#define u64 uint64_t

#define i32 int32_t

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

#define INIT_ZOOM 200

#define OFFSET_X -2
#define OFFSET_Y -1

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define CLAMP(val, min, max) (MAX(min, MIN(val, max)))

#define MAPVAL(val, min_original, max_original, min_new, max_new)              \
    (((val) - (min_original)) / ((max_original) - (min_original))) *           \
            ((max_new) - min_new) +                                            \
        (min_new)

f32 lerpf(f32 a, f32 b, f32 t) { return a * (1 - t) + b * t; }

SDL_Color color_lerp(SDL_Color a, SDL_Color b, f32 t) {
    return (SDL_Color){
        lerpf(a.r, b.r, t),
        lerpf(a.g, b.g, t),
        lerpf(a.b, b.b, t),
        lerpf(a.a, b.a, t),
    };
}

#define INIT_MAX_ITERATIONS 50

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

static i32 g_captures_this_frame = 0;

void ui_capture_input(void) { g_captures_this_frame++; }

void ui_release_input(void) { g_captures_this_frame++; }

b32 ui_focused(void) { return g_captures_this_frame > 0; }

// Slider vocabulary:
// https://developer.apple.com/design/human-interface-guidelines/sliders
//
//  --[]-- <- pretend this is a slider
//
//  ------ <- this is the track
//    []   <- this is the thumb
typedef struct Slider {
    Vector2 pos;
    SDL_FRect track;
    SDL_FRect thumb;
    f32 ui_min;
    f32 ui_max;
    f32 val_min;
    f32 val_max;
    b32 sliding;
} Slider;

Slider slider_create(Vector2 pos, f32 val_min, f32 val_max, f32 val_init) {
    Slider s = {0};
    s.pos = pos;
    s.track = (SDL_FRect){pos.x, pos.y, 100, 2};
    s.thumb = (SDL_FRect){pos.x, pos.y - 4, 5, 10};
    s.ui_min = pos.x;
    s.ui_max = pos.x + s.track.w - s.thumb.w / 2.0f;
    s.val_min = val_min;
    s.val_max = val_max;
    s.sliding = FALSE;

    s.thumb.x = MAPVAL(val_init, val_min, val_max, s.ui_min, s.ui_max);

    return s;
}

void slider_button_down(Slider* s, f32 x, f32 y) {
    if (SDL_PointInRectFloat(&(SDL_FPoint){x, y}, &s->thumb)) {
        s->sliding = TRUE;
        ui_capture_input();
    }
}

void slider_button_up(Slider* s) {
    s->sliding = FALSE;
    ui_release_input();
}

void slider_update(Slider* s, f32* val, Vector2 pos) {
    if (!s->sliding) {
        return;
    }
    s->thumb.x = CLAMP(pos.x, s->ui_min, s->ui_max);
    *val = MAPVAL(s->thumb.x, s->ui_min, s->ui_max, s->val_min, s->val_max);
}

void slider_draw(Slider* s, SDL_Renderer* renderer) {
    SDL_RenderFillRect(renderer, &s->track);
    SDL_RenderFillRect(renderer, &s->thumb);
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

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
    f32 camera_zoom = INIT_ZOOM;
    f32 camera_zoom_sensitivity = 0.0;
    f32 scroll_wheel = 0.0f;

    b8 mouse_dragging = FALSE;
    Vector2 mouse_pos = {0.0, 0.0};
    Vector2 last_mouse_pos = {0.0, 0.0};

    f32 max_iterations = INIT_MAX_ITERATIONS;
    char max_iterations_text_buffer[32];

    Slider max_iterations_slider =
        slider_create((Vector2){10, 45}, 1, 100, max_iterations);

    f32 cx = 0.5;
    f32 cy = 0.5;
    Slider cx_slider = slider_create((Vector2){10, 55}, -1, 1, cx);
    Slider cy_slider = slider_create((Vector2){10, 65}, -1, 1, cy);

// TODO(novel): Add delta time

// https://coolors.co/fde8e9-e3bac6-bc9ec1-596475-1f2232
#define PALETTE_COUNT 8
    SDL_Color palette[PALETTE_COUNT] = {
        {31, 34, 50, 255},    {89, 100, 117, 255},  {188, 158, 193, 255},
        {227, 186, 198, 255}, {253, 232, 233, 255}, {227, 186, 198, 255},
        {188, 158, 193, 255}, {89, 100, 117, 255}};

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
                if (event.key.scancode == SDL_SCANCODE_Q) {
                    app_running = FALSE;
                }
                break;
            case SDL_EVENT_MOUSE_WHEEL: {
                scroll_wheel = event.wheel.y;
            } break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    slider_button_down(&max_iterations_slider, event.button.x,
                                       event.button.y);
                    slider_button_down(&cx_slider, event.button.x,
                                        event.button.y);
                    slider_button_down(&cy_slider, event.button.x,
                                       event.button.y);
                }
            } break;
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    slider_button_up(&max_iterations_slider);
                    slider_button_up(&cx_slider);
                    slider_button_up(&cy_slider);
                }
            } break;
            default:
                break;
            }
        }

        last_mouse_pos = mouse_pos;
        mouse_dragging =
            SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y) & SDL_BUTTON_LMASK;

        if (mouse_dragging && (!ui_focused())) {
            Vector2 world_pos =
                screen_to_world(mouse_pos, camera_pos, camera_zoom);
            Vector2 last_world_pos =
                screen_to_world(last_mouse_pos, camera_pos, camera_zoom);

            // printf("X: %.2f, Y: %.2f\n", world_pos.x, world_pos.y);
            // fflush(stdout);

            Vector2 mouse_world_diff = vector2_sub(last_world_pos, world_pos);
            camera_pos = vector2_add(camera_pos, mouse_world_diff);
        }

        slider_update(&max_iterations_slider, &max_iterations, mouse_pos);
        slider_update(&cx_slider, &cx, mouse_pos);
        slider_update(&cy_slider, &cy, mouse_pos);

        camera_zoom_sensitivity = fabs(camera_zoom) / 10;
        if (scroll_wheel > VERY_SMALL) {
            camera_zoom += camera_zoom_sensitivity;
            // printf("Zoom: %.2f\n", camera_zoom);
            // fflush(stdout);
        } else if (scroll_wheel < -VERY_SMALL) {
            camera_zoom -= camera_zoom_sensitivity;
            // printf("Zoom: %.2f\n", camera_zoom);
            // fflush(stdout);
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
        snprintf(max_iterations_text_buffer, sizeof(max_iterations_text_buffer),
                 "Max Iterations: %.0f", max_iterations);

        f32 R = 4;
        Vector2 C = {cx, cy};
        for (int y = 0; y < TEXTURE_HEIGHT; y++) {
            for (int x = 0; x < TEXTURE_WIDTH; x++) {
                Vector2 z =
                    screen_to_world((Vector2){x, y}, camera_pos, camera_zoom);

                f32 iteration = 0;

                // https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Optimized_escape_time_algorithms
                while ((z.x * z.x + z.y * z.y <= R * R) &&
                       (iteration < max_iterations)) {

                    f32 xtemp = z.x * z.x - z.y * z.y;
                    z.y = 2 * z.x * z.y + C.y;
                    z.x = xtemp + C.x;
                    iteration += 1.0;
                }

                // https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Continuous_(smooth)_coloring
                if (iteration < max_iterations) {
                    f32 logzn = logf(z.x * z.x + z.y * z.y) / 2.0;
                    f32 nu = logf(logzn / logf(2)) / (logf(2));
                    iteration = iteration + 1 - nu;
                } else {
                    u8 r = palette[0].r;
                    u8 g = palette[0].g;
                    u8 b = palette[0].b;

                    pixel_buffer[y * TEXTURE_WIDTH + x] =
                        0xFF000000 | ((u32)r << 16) | ((u32)g << 8) | b;
                    continue; // color fixed if no escape
                }

                // https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Exponentially_mapped_and_cyclic_iterations
                f32 s = 0.5;
                i32 n = PALETTE_COUNT;
                f32 t = powf(iteration / (f32)max_iterations, s) * n;
                f32 tpow1_5 = powf(t, 1.5f);
                i32 v = (i32)tpow1_5;
                f32 blend = tpow1_5 - floorf(tpow1_5);
                SDL_Color c1 = palette[v % PALETTE_COUNT];
                SDL_Color c2 = palette[(v + 1) % PALETTE_COUNT];
                SDL_Color c = color_lerp(c1, c2, blend);

                u8 r = c.r;
                u8 g = c.g;
                u8 b = c.b;

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

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderDebugText(renderer, 10.0f, 10.0f, frame_time_text_buffer);
        SDL_RenderDebugText(renderer, 10.0f, 20.0f, fps_text_buffer);
        SDL_RenderDebugText(renderer, 10.0f, 30.0f, max_iterations_text_buffer);

        slider_draw(&max_iterations_slider, renderer);
        slider_draw(&cx_slider, renderer);
        slider_draw(&cy_slider, renderer);

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
