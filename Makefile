CC      	?= gcc
RELEASE 	?= 0
LIBS 		= -lncurses

ifeq ($(RELEASE), 1)
	CFLAGS   = -std=gnu11 -Wall -Wextra -Wpedantic -O2 -MMD -MP -Iinclude
	LDFLAGS  = $(LIBS)
else
	CFLAGS  = -std=gnu11 -Wall -Wextra -Wpedantic -g3 -O0 \
              -fsanitize=address,undefined -MMD -MP -Iinclude
    LDFLAGS = -fsanitize=address,undefined -lncurses
endif

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
