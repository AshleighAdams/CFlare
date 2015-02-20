
include config

.PHONY: default all clean run

default: $(TARGET)
all: $(TARGET)

%.o: %.c $(LIB_HEADERS)
	$(CC) $(CFLAGS) $(PIC) -c $< -o $@

.PRECIOUS: lib$(TARGET) $(LIB_OBJECTS)

lib$(TARGET): $(LIB_OBJECTS)
	$(CC) $(CFLAGS) -shared $(LIB_OBJECTS) $(LFLAGS) $(LIBS) -o "$@.$(LIB_EXTENSION)"

$(TARGET): lib$(TARGET) $(EXE_HEADERS) $(EXE_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -l$(TARGET) $(LFLAGS) $(LIBS) -o "$@" $(EXE_SOURCES)

clean:
	@echo $(LIB_OBJECTS)
	-$(RM) "lib$(TARGET).$(LIB_EXTENSION)" "$(TARGET)" $(wildcard src/**.o) $(wildcard src/**/*.o)

test:
	@ \
	./cflare --version; \
	echo -n "testing for tabs as allignment... "; \
	OUTPUT=$$(grep --include "*.h" --include "*.c" "	[^	].*	" -r src); \
	RETURN=$$?; \
	if [ "$$RETURN" = "0" ]; then \
		echo "fail"; \
		echo $$OUTPUT; \
		exit 1; \
	else \
		echo "okay"; \
	fi;
	@echo "performing unit tests..."
	./cflare unit-test;
	@echo "testing for memory leaks..."
	valgrind --leak-check=full --error-exitcode=1 ./cflare unit-test > /dev/null
