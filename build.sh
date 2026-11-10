#!/bin/sh

mkdir -p build
clang -std=c11 -Wall -Wextra -Wpedantic -g -lSDL3 -lm src/mandelbrot.c -o build/mandelbrot
clang -std=c11 -Wall -Wextra -Wpedantic -g -lSDL3 -lm src/julia.c -o build/julia
