
TARGET=cflare
CC ?= gcc
CFLAGS = -g -Wall -Isrc/ -std=c11 -D_GNU_SOURCE
LFLAGS = -Wall -std=c11
LIBS = -lm

LFLAGS += -lpthread

.PHONY: default all clean

default: $(TARGET)
all: $(TARGET)

OBJECTS = $(patsubst %.c, %.o, $(wildcard src/**/*.c) $(wildcard src/*.c))
HEADERS = $(wildcard src/**/*.h) $(wildcard src/*.h)

%.o: %.c $(HEADERS)
	$(CC) $(C_FLAGS) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) $(LIBS) -o $@

clean:
	@echo $(OBJECTS)
	-$(RM) $(TARGET) $(wildcard src/**.o) $(wildcard src/**/*.o)
