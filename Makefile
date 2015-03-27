default:

include config

.PHONY: default all clean run test test-additional

default: $(TARGET)
all: $(TARGET)

%.o: %.c $(LIB_HEADERS)
	$(CC) $(CFLAGS) $(PIC) -c $< -o $@

.PRECIOUS: lib$(TARGET) $(LIB_OBJECTS) $(EXE_OBJECTS)

lib$(TARGET): $(LIB_OBJECTS)
	$(CC) $(CFLAGS) -shared $(LIB_OBJECTS) $(LFLAGS) $(LIBS) -o "$@.$(LIB_EXTENSION)"

$(TARGET): lib$(TARGET) $(EXE_OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -l$(TARGET) $(LFLAGS) $(LIBS) $(EXE_OBJECTS) -o "$@"

clean:
	-$(RM) "lib$(TARGET).$(LIB_EXTENSION)" "$(TARGET)" \
		$(wildcard src/**.o) $(wildcard src/**/*.o) \
		$(wildcard src/**.gcda) $(wildcard src/**/*.gcda) \
		$(wildcard src/**.gcno) $(wildcard src/**/*.gcno) \

test: 
	$(MAKE) -j1 test-units test-memory test-tabs test-headers test-exports test-static-analysis tests-additional
	@echo "All tests passed for `./cflare --version`";

test-exports:
	@echo "checking all exports of lib$(TARGET).$(LIB_EXTENSION) are valid..."
	! nm "lib$(TARGET).$(LIB_EXTENSION)" | grep -v " T cflare_" | grep " T [a-zA-Z]"

test-headers:
	@echo "checking all headers are self sufficient..."; \
	for HEADER in $$(find ./src/ -name "*.h"); do \
		echo "#include \"$$HEADER\"\nint main(){return 0;}" > ./.tmp.c; \
		echo "\t$$HEADER...\033[s"; \
		$(CC) -std=c11 -I./src ./.tmp.c -o /dev/null; \
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

test-static-analysis:
	@ if whatis cppcheck 1> /dev/null 2> /dev/null; then \
		echo "performing static analysis..."; \
		cppcheck --std=c11 --quiet --inconclusive --enable=all -I src/ src/; \
	else \
		echo "can't do static analysis: cppcheck not found."; \
	fi;

test-units:
	@echo "performing unit tests..."; \
	./cflare unit-test;

test-coverage:
	@echo "coverage information:"
	gcovr -r src/
