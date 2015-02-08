
TARGET=cflare
CC ?= gcc
CFLAGS = -g -Wall -Isrc/ -std=c11 -D_GNU_SOURCE
LFLAGS = -Wall -std=c11
LIBS = -lm -lpthread

.PHONY: default all clean run

default: $(TARGET)
all: $(TARGET)

LIB_OBJECTS = $(patsubst %.c, %.o, $(wildcard src/cflare/**/*.c) $(wildcard src/cflare/*.c))
LIB_HEADERS = $(wildcard src/cflare/**/*.h) $(wildcard src/cflare/*.h)

EXE_SOURCES = $(wildcard src/*.c)
EXE_HEADERS = $(wildcard src/*.h)

%.o: %.c $(LIB_HEADERS)
	$(CC) -fPIC $(CFLAGS) -c $< -o $@

.PRECIOUS: lib$(TARGET) $(LIB_OBJECTS)

lib$(TARGET): $(LIB_OBJECTS)
	$(CC) -shared $(LIB_OBJECTS) $(LFLAGS) $(LIBS) -o "$@.so"

$(TARGET): lib$(TARGET) $(EXE_HEADERS) $(EXE_SOURCES)
	$(CC) -L. -l$(TARGET) $(CFLAGS) $(LFLAGS) $(LIBS) -o "$@" $(EXE_SOURCES)

clean:
	@echo $(LIB_OBJECTS)
	-$(RM) "lib$(TARGET).so" $(wildcard src/**.o) $(wildcard src/**/*.o)

run:
	LD_LIBRARY_PATH=".:$LD_LIBRARY_PATH" ./cflare
