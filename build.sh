#!/bin/sh

mkdir -p build
clang -std=c11 -Wall -Wextra -Wpedantic -g -lSDL3 -lm src/main.c -o build/visuals
