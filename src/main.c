#include <SDL3/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define b32 int32_t

#define TRUE 1
#define FALSE 0

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Could not initalize SDL: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Window* window = SDL_CreateWindow("Mandelbrot", 800, 800, 0);
    if (window == NULL) {
        printf("Could not create window: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL) {
        printf("Could not create renderer: %s", SDL_GetError());
        return EXIT_FAILURE;
    }

    b32 app_running = TRUE;

    SDL_Event event;

    while (app_running) {
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

        // render
        SDL_RenderClear(renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
