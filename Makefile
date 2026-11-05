CC = cc
CFLAGS = -std=c11 -Wall -g
LDFLAGS =
LDLIBS = -lSDL3 -lm

BUILD_DIR = build
TARGET = $(BUILD_DIR)/visuals

SRCS = src/main.c
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
