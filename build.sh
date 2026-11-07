#!/bin/sh

mkdir -p build
clang -std=c11 -Wall -g -lSDL3 -lm src/main.c -o build/visuals
