CC      ?= gcc
CFLAGS   = -std=gnu11 -Wall -Wextra -Wpedantic -g3 -O0 \
           -fsanitize=address,undefined -MMD -MP -Iinclude
LDFLAGS  = -fsanitize=address,undefined -lncurses

SRCS     = $(shell find src -name '*.c')
OBJS     = $(patsubst src/%.c, build/%.o, $(SRCS))
DEPS     = $(OBJS:.o=.d)
TARGET   = cantop

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build $(TARGET)

-include $(DEPS)

.PHONY: all clean
