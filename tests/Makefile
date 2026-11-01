# Headless tests for raylib (no window, no GPU required to RUN them)
#
# USAGE (from the repository root of the raylib fork, after building src/libraylib.a):
#     cd src && make PLATFORM=PLATFORM_DESKTOP     (mingw32-make on Windows/MSYS2)
#     cd ../tests && make test
#
# VARIABLES:
#     RAYLIB_SRC   folder with raylib.h and libraylib.a   (default: ../src)

RAYLIB_SRC ?= ../src
CC         ?= gcc
CFLAGS     ?= -std=c99 -Wall -Wextra -Wno-unused-function -I$(RAYLIB_SRC)

ifeq ($(OS),Windows_NT)
    EXE  = .exe
    LIBS = -L$(RAYLIB_SRC) -lraylib -lopengl32 -lgdi32 -lwinmm
else
    EXE  =
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        LIBS = -L$(RAYLIB_SRC) -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    else
        LIBS = -L$(RAYLIB_SRC) -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif
endif

TESTS = test_hash test_paths test_utf8

all: $(addsuffix $(EXE),$(TESTS))

%$(EXE): %.c rtest.h
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

# Runs every test binary; stops at the first failing one (non-zero exit code)
test: all
	@for t in $(TESTS); do echo "=== $$t ==="; ./$$t$(EXE) || exit 1; done
	@echo "ALL TEST FILES PASSED"

clean:
	rm -f $(addsuffix $(EXE),$(TESTS))

.PHONY: all test clean
