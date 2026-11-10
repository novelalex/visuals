COMPILER=clang
OPTIONS=-std=c11 -Wall -Wextra -Wpedantic -g
LIBS=-lSDL3 -lm
BUILD=build

COMPILE=$(COMPILER) $(OPTIONS) $(LIBS)

all: $(BUILD)/mandelbrot $(BUILD)/julia

$(BUILD)/mandelbrot: src/mandelbrot.c | build
	$(COMPILE) $< -o $@

$(BUILD)/julia: src/julia.c | build
	$(COMPILE) $< -o $@

build:
	mkdir -p $(BUILD)

clean:
	rm -r $(BUILD)

.PHONY: build clean
