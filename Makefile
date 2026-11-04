CC = cc
CFLAGS = -std=c11 -Wall -g
LDFLAGS =
LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

BUILD_DIR = build
TARGET = $(BUILD_DIR)/visuals

SRCS = $(shell find src -name '*.c')
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
