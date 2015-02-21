
include config

.PHONY: default all clean run test

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
	-$(RM) "lib$(TARGET).$(LIB_EXTENSION)" "$(TARGET)" $(wildcard src/**.o) $(wildcard src/**/*.o)

test: test-units test-memory test-tabs test-headers
	echo "All tests passed for `./cflare --version`";

test-headers:
	@echo "checking all headers are self sufficient..."; \
	for HEADER in $$(find ./src/ -name "*.h"); do \
		echo "#include \"$$HEADER\"\nint main(){return 0;}" > ./.tmp.c; \
		echo "\t$$HEADER...\033[s"; \
		$(CC) -I./src ./.tmp.c -o /dev/null; \
		if [ "$$?" != "0" ]; then \
			rm ./.tmp.c; \
			exit 1; \
		else \
			echo "\033[u\033[1A okay"; \
		fi; \
	done; \
	rm ./.tmp.c;

test-tabs:
	@echo -n "testing for tabs as allignment... "; \
	OUTPUT=$$(grep --include "*.h" --include "*.c" "	[^	].*	" -r src); \
	RETURN=$$?; \
	if [ "$$RETURN" = "0" ]; then \
		echo "fail"; \
		echo $$OUTPUT; \
		exit 1; \
	else \
		echo "okay"; \
	fi;

test-memory:
	@echo "testing for memory leaks..."; \
	valgrind --leak-check=full --error-exitcode=1 ./cflare unit-test > /dev/null

test-units:
	@echo "performing unit tests..."; \
	./cflare unit-test;
