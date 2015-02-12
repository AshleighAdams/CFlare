
TARGET=cflare
CC ?= gcc
CFLAGS += -Wall -Isrc/ -std=c11 -D_GNU_SOURCE
LFLAGS += -Wall -std=c11
LIBS += -lm -lpthread
LDFLAGS += -L.

DEBUG ?= 1

ifeq ($(DEBUG), 1)
	ifeq ($(shell uname -s), Linux)
		LDFLAGS += -Wl,-R -Wl,.
	endif
	CFLAGS += -g
else
	CFLAGS += -O3
endif

.PHONY: default all clean run

default: $(TARGET)
all: $(TARGET)

LIB_OBJECTS = $(patsubst %.c, %.o, $(wildcard src/cflare/**/*.c) $(wildcard src/cflare/*.c))
LIB_HEADERS = $(wildcard src/cflare/**/*.h) $(wildcard src/cflare/*.h)

EXE_SOURCES = $(wildcard src/*.c)
EXE_HEADERS = $(wildcard src/*.h)

ifeq ($(OS),Windows_NT)
	LIB_OBJECTS += $(patsubst %.c, %.o, $(wildcard src/cflare-windows/**/*.c) $(wildcard src/cflare-windows/*.c))
	LIB_HEADERS += $(wildcard src/cflare-windows/**/*.h) $(wildcard src/cflare-windows/*.h)
	LIB_EXTENSION ?= dll
else
	LIB_OBJECTS += $(patsubst %.c, %.o, $(wildcard src/cflare-posix/**/*.c) $(wildcard src/cflare-posix/*.c))
	LIB_HEADERS += $(wildcard src/cflare-posix/**/*.h) $(wildcard src/cflare-posix/*.h)
	ifeq ($(shell uname -s), Darwin)
		LIB_EXTENSION ?= dylib
	else
		LIB_EXTENSION ?= so
	endif
endif

%.o: %.c $(LIB_HEADERS)
	$(CC) -fPIC $(CFLAGS) -c $< -o $@

.PRECIOUS: lib$(TARGET) $(LIB_OBJECTS)

lib$(TARGET): $(LIB_OBJECTS)
	$(CC) -shared $(LIB_OBJECTS) $(LFLAGS) $(LIBS) -o "$@.$(LIB_EXTENSION)"

$(TARGET): lib$(TARGET) $(EXE_HEADERS) $(EXE_SOURCES)
	$(CC) $(LDFLAGS) -l$(TARGET) $(CFLAGS) $(LFLAGS) $(LIBS) -o "$@" $(EXE_SOURCES)

clean:
	@echo $(LIB_OBJECTS)
	-$(RM) "lib$(TARGET).$(LIB_EXTENSION)" "$(TARGET)" $(wildcard src/**.o) $(wildcard src/**/*.o)

