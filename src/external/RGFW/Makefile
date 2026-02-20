# CUSTOM ARGS :
# WAYLAND=1 -> use wayland
# WAYLAND_X11=1 -> use wayland AND X11
# NO_VULKAN=1 -> do not compile the vulkan example
# NO_GLES=1 -> do not compile the gles example (on by default for non-linux OSes)
# NO_OSMESA=1 -> do not compile the osmesa example (on by default for non-linux OSes)

CC ?= gcc
AR ?= ar

# used for compiling RGFW.o
CUSTOM_CFLAGS =
# used for the examples
CFLAGS =
CFLAGS += -g3

ifdef RGFW_C89
	CUSTOM_CFLAGS += -std=c89 -Wno-declaration-after-statement -Wall -Wextra -Wpedantic
endif

DX11_LIBS = -static -lgdi32 -ldxgi -ld3d11 -luuid -ld3dcompiler
VULKAN_LIBS = -lgdi32 -I $(VULKAN_SDK)\Include -L $(VULKAN_SDK)\Lib -lvulkan-1
LIBS := -static -lgdi32
LINK_GL1 = -lopengl32
EXT = .exe
LIB_EXT = .dll

ifneq ($(CC),zig cc)
	LIBS += -D _WIN32_WINNT=0x0501
endif

OS_DIR = \\

NO_OSMESA = 1
NO_GLES = 1
NO_EGL = 1
detected_OS = windows

OBJ_FILE = .o

ifeq ($(WAYLAND_ONLY), 1)
	WAYLAND = 1
endif

ifeq ($(WAYLAND_X11), 1)
	WAYLAND = 1
endif

# not using a cross compiler
ifeq (,$(filter $(CC),x86_64-w64-mingw32-gcc i686-w64-mingw32-gcc x86_64-w64-mingw32-g++ /opt/msvc/bin/x64/cl.exe /opt/msvc/bin/x86/cl.exe))
	detected_OS := $(shell uname 2>/dev/null || echo Unknown)

	ifeq ($(detected_OS),Darwin)        # Mac OS X
		DX11_LIBS =
		LIBS := -framework CoreVideo -framework Cocoa -framework OpenGL -framework IOKit
		LINK_GL1 = -framework OpenGL
		VULKAN_LIBS = $(LIBS) -lvulkan
		EXT =
		LIB_EXT = .dylib
		OS_DIR = /
		NO_VULKAN ?= 1
	endif
	ifeq ($(detected_OS),Linux)
		DX11_LIBS =
		LINK_GL1 = -lGL

		ifneq ($(WAYLAND_ONLY), 1)
			LIBS = -lXrandr -lX11 -ldl -lpthread -lm
		else
			LIBS =
		endif

		VULKAN_LIBS = $(LIBS) -lvulkan
		EXT =
		LIB_EXT = .so
		OS_DIR = /
		NO_GLES = 0
		NO_EGL = 0
	endif
	ifeq ($(detected_OS),NetBSD)
		DX11_LIBS =
		LINK_GL1 = -lGL
		CUSTOM_CFLAGS += -I/usr/pkg/include -I/usr/X11R7/include -Wl,-R/usr/pkg/lib -Wl,-R/usr/X11R7/lib -L/usr/pkg/lib -L/usr/X11R7/lib
		LIBS := $(CUSTOM_CFLAGS) -lXrandr -lX11 -lpthread -lm
		VULKAN_LIBS = $(LIBS) -lvulkan
		EXT =
		LIB_EXT = .so
		OS_DIR = /
		NO_GLES = 0
		NO_EGL = 0
		NO_VULKAN = 1
	endif

	ifeq (,$(filter $(detected_OS),Linux Darwin NetBSD))
		detected_OS := windows
	endif
else
	OS_DIR = /
endif

ifeq ($(WAYLAND),1)
	WAYLAND_SOURCE = $(shell make -f wayland.mk sources -s)

	NO_VULKAN = 1
	NO_GLES = 0
	NO_EGL = 0
	LIBS += -D RGFW_WAYLAND $(WAYLAND_SOURCE) -lwayland-cursor -lwayland-client -lxkbcommon  -lwayland-egl -lEGL -lm
	LINK_GL1 = -lEGL -lGL

	# LIBS += -ldecor-0

	ifeq ($(WAYLAND_X11), 1)
		LIBS += -D RGFW_X11
	endif
endif

LINK_GL3 =
LINK_GL2 =
LINK_OSMESA =

ifneq (,$(filter $(CC),cl /opt/msvc/bin/x64/cl.exe /opt/msvc/bin/x86/cl.exe))
	WARNINGS = -Wall -wd4668 -wd4820 -wd5045
	LIBS =
	DX11_LIBS =
	VULKAN_LIBS =
	OBJ_FILE = .obj
else ifneq (,$(filter $(CC),emcc em++))
	DX11_LIBS =
	LINK_GL1 = -s LEGACY_GL_EMULATION -D LEGACY_GL_EMULATION -sGL_UNSAFE_OPTS=0
	LINK_GL3 = -s FULL_ES3 -s USE_WEBGL2
	LINK_GL2 = -s FULL_ES2 -s USE_WEBGL2
	LINK_OSMESA = -sALLOW_MEMORY_GROWTH
	EXPORTED_JS = -s EXPORTED_RUNTIME_METHODS="['stringToNewUTF8']"
	LIBS = -s WASM=1 -s ASYNCIFY -s GL_SUPPORT_EXPLICIT_SWAP_CONTROL=1 $(EXPORTED_JS)
	EXT = .js
	NO_GLES = 0
	NO_EGL = 0
	NO_VULKAN = 1
	detected_OS = web
	DX11_LIBS =
else ifeq (,$(filter $(CC),g++ clang++ em++))
	CFLAGS += -std=c99
	WARNINGS = -Wall -Werror -Wdouble-promotion -Wmissing-prototypes -Wextra -Wstrict-prototypes -Wold-style-definition -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wvla -Wcast-align -Wstrict-overflow -Wnested-externs -Wstrict-aliasing -Wredundant-decls -Winit-self -Wmissing-noreturn
else
	WARNINGS = -Wall -Werror -Wdouble-promotion -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wvla -Wcast-align -Wstrict-overflow -Wstrict-aliasing -Wredundant-decls -Winit-self -Wmissing-noreturn

	NO_VULKAN = 1
	ifeq ($(detected_OS),Darwin)
		WARNINGS += -Wno-deprecated -Wno-unknown-warning-option -Wno-pedantic
	endif
endif

ifeq ($(WAYLAND_X11), 1)
	# lazy fix (no -Wmissing-prototypes for wayland X11)
	WARNINGS = -Wall -Werror -Wdouble-promotion -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wvla -Wcast-align -Wstrict-overflow -Wstrict-aliasing -Wredundant-decls -Winit-self -Wmissing-noreturn
endif

EXAMPLE_OUTPUTS = \
	examples/gl11/gl11 \
	examples/surface/surface\
	examples/event_queue/event_queue \
	examples/callbacks/callbacks \
	examples/state-checking/state-checking \
	examples/flags/flags \
	examples/monitor/monitor \
	examples/gamma/gamma \
	examples/gl33_ctx/gl33_ctx \
	examples/smooth-resize/smooth-resize \
	examples/multi-window/multi-window \
	examples/standard-mouse-icons/icons \
	examples/custom_alloc/custom_alloc \
	examples/flash/flash \

EXAMPLE_OUTPUTS_CUSTOM = \
	examples/window_icons/icons \
	examples/mouse_icons/icons \
	examples/gamepad/gamepad \
	examples/first-person-camera/camera \
	examples/microui_demo/microui_demo \
	examples/gl33/gl33 \
	examples/portableGL/pgl \
	examples/gles2/gles2 \
	examples/egl/egl \
	examples/osmesa_demo/osmesa_demo \
	examples/vk10/vk10 \
	examples/dx11/dx11 \
	examples/metal/metal \
	examples/minimal_links/minimal_links \
	examples/gears/gears \
	examples/srgb/srgb

TEST_OUTPUTS = \
			   tests/loop \


all: $(WAYLAND_SOURCE) $(EXAMPLE_OUTPUTS) $(EXAMPLE_OUTPUTS_CUSTOM) libRGFW$(LIB_EXT) libRGFW.a

examples: $(WAYLAND_SOURCE) $(EXAMPLE_OUTPUTS) $(EXAMPLE_OUTPUTS_CUSTOM)

tests: $(WAYLAND_SOURCE) $(TEST_OUTPUTS)

examples/gears/gears: examples/gears/gears.c RGFW.h $(WAYLAND_SOURCE)
ifneq (,$(filter $(CC),emcc em++))
	@echo gears is not supported on this platform
else
	$(CC) $(CFLAGS) -I. $< $(LINK_GL1) $(LIBS) -lm $($)  -o $@$(EXT)
endif

examples/srgb/srgb: examples/srgb/srgb.c RGFW.h $(WAYLAND_SOURCE)
	$(CC) $(CFLAGS) -I. $< $(LINK_GL1) $(LIBS) -lm $($)  -o $@$(EXT)

examples/portableGL/pgl: examples/portableGL/pgl.c RGFW.h $(WAYLAND_SOURCE)
ifeq (,$(filter $(CC),emcc em++))
	$(CC)  -w $(CFLAGS) -I. $< -lm $(LIBS) -o $@
else
	@echo "the portableGL example doesn't support html5"
endif

examples/gles2/gles2: examples/gles2/gles2.c RGFW.h $(WAYLAND_SOURCE)
ifneq ($(NO_GLES), 1)
	$(CC)  $(CFLAGS) -I. $< $(LIBS) $(LINK_GL2) -lGL -o $@$(EXT)
else
	@echo gles has been disabled
endif

examples/egl/egl: examples/egl/egl.c RGFW.h $(WAYLAND_SOURCE)
ifneq ($(NO_EGL), 1)
	$(CC)  $(CFLAGS) -I. $< $(LIBS) $(LINK_GL1) -lGL -lEGL -o $@$(EXT)
else
	@echo egl has been disabled
endif



examples/osmesa_demo/osmesa_demo: examples/osmesa_demo/osmesa_demo.c RGFW.h $(WAYLAND_SOURCE)
ifneq ($(NO_OSMESA), 1)
	$(CC)  $(CFLAGS) -I. $< $(LIBS) $(LINK_OSMESA) -lOSMesa -o $@$(EXT)
else
	@echo osmesa has been disabled
endif


examples/vk10/vk10: examples/vk10/vk10.c examples/vk10/vkinit.h RGFW.h $(WAYLAND_SOURCE)
ifneq ($(NO_VULKAN), 1)
	glslangValidator -V examples/vk10/shaders/vert.vert -o examples/vk10/shaders/vert.h --vn vert_code
	glslangValidator -V examples/vk10/shaders/frag.frag -o examples/vk10/shaders/frag.h --vn frag_code

	$(CC)  $(CFLAGS) -I. $< $(VULKAN_LIBS) -o $@
else
	@echo vulkan has been disabled
endif

examples/every_api/every:
ifneq ($(NO_VULKAN), 1)
	$(CC)  $(CFLAGS) -I. $< $(LINK_GL1) $(VULKAN_LIBS) $(DX11_LIBS) -o $@
else
	$(CC)  $(CFLAGS) -I. $< $(LIBS) $(LINK_GL1) $(DX11_LIBS) -D RGFW_NO_VULKAN -o $@
endif

examples/dx11/dx11: examples/dx11/dx11.c RGFW.h $(WAYLAND_SOURCE)
ifeq ($(CC), zig cc)
	@echo directX is not supported with Zig
else ifneq (,$(filter $(CC),g++ clang++ "zig cc"))
	@echo directX is not supported with C++
else ifneq (,$(filter $(detected_OS), windows Windows_NT))
	$(CC) $(CFLAGS) -I. $<  $(DX11_LIBS) -o $@
else
	@echo directX is not supported on $(detected_OS)
endif


examples/metal/metal: examples/metal/metal.m RGFW.h $(WAYLAND_SOURCE)
ifeq ($(detected_OS),Darwin)        # Mac OS X
	$(CC) $(CFLAGS) examples/metal/metal.m -I. -framework CoreVideo -framework Metal -framework Cocoa -framework IOKit -framework QuartzCore -o $@
else
	@echo metal is not supported on $(detected_OS)
endif

examples/minimal_links/minimal_links: examples/minimal_links/minimal_links.c RGFW.h $(WAYLAND_SOURCE)
ifeq ($(WAYLAND), 1)
	@echo nostl is not supported on this platform
else ifneq (,$(filter $(CC),emcc em++))
	@echo nostl is not supported on this platform
else ifeq ($(detected_OS),NetBSD)
	$(CC) $(CFLAGS) $(CFLAGS) -pthread -I. $<  -o $@$(EXT)
else ifeq ($(detected_OS),Linux)
	$(CC) $(CFLAGS) -I. $< -lXrandr -lX11 -o $@$(EXT)
else ifeq ($(detected_OS),windows)
	$(CC) $(CFLAGS) $(WARNINGS) -I. $< -lgdi32 -o $@$(EXT)
else ifeq ($(detected_OS),Darwin)
	$(CC) $(CFLAGS) $(WARNINGS) -I. $< -framework CoreVideo -framework Cocoa  -o $@$(EXT)
else
	@echo minimal_links is not supported on this platform
endif


examples/nostl/nostl: examples/nostl/nostl.c RGFW.h $(WAYLAND_SOURCE)
ifeq ($(WAYLAND), 1)
	@echo nostl is not supported on this platform
else ifneq (,$(filter $(CC),emcc em++))
	@echo nostl is not supported on this platform
else ifeq ($(detected_OS),Linux)
	$(CC) $(CFLAGS) -fno-stack-protector -lX11 -lXcursor -lGL -lXi -lXrandr -I. $<  -o $@$(EXT)
else ifeq ($(detected_OS),windows)
	$(CC) $(CFLAGS) $(WARNINGS) -I. $<  -lkernel32 -lgdi32 -lshell32 -lUser32 -o $@$(EXT)
else ifeq ($(detected_OS),Darwin)
	$(CC) $(CFLAGS) -fno-stack-protector  $(WARNINGS) -I. $< -framework CoreVideo -framework Cocoa -o $@$(EXT)
else
	@echo nostl is not supported on this platform
endif


examples/microui_demo/microui_demo: examples/microui_demo/microui_demo.c RGFW.h $(WAYLAND_SOURCE)
ifeq (,$(filter $(CC),emcc em++ g++ clang++))
	$(CC) $(CFLAGS) -I. $< examples/microui_demo/microui.c  $(LINK_GL1) $(LIBS) -o $@$(EXT)
else ifneq (,$(filter $(CC),em++ g++ clang++))
	@echo microui demo not supported with C++
else
	$(CC) $(CFLAGS) -I. $< examples/microui_demo/microui.c -s USE_WEBGL2 $(LIBS) $(LINK_GL1) -o $@$(EXT)
endif

examples/window_icons/icons: examples/window_icons/icons.c RGFW.h $(WAYLAND_SOURCE)
	$(CC) $(CFLAGS) -I. $< $(LIBS) -lm $(LINK_GL1) -o $@$(EXT)
examples/mouse_icons/icons: examples/mouse_icons/icons.c RGFW.h $(WAYLAND_SOURCE)
	$(CC) $(CFLAGS) -I. $< $(LIBS) -lm $(LINK_GL1) -o $@$(EXT)
examples/gamepad/gamepad: examples/gamepad/gamepad.c RGFW.h $(WAYLAND_SOURCE)
	$(CC) $(CFLAGS) -I. $< $(LIBS) -lm $(LINK_GL1) -o $@$(EXT)

examples/first-person-camera/camera: examples/first-person-camera/camera.c RGFW.h $(WAYLAND_SOURCE)
	$(CC) $(CFLAGS) -I. $< $(LIBS) -lm $(LINK_GL1) -o $@$(EXT)


examples/gl33/gl33: examples/gl33/gl33.c RGFW.h $(WAYLAND_SOURCE)
ifeq ($(WAYLAND), 1)
	$(CC) $(CFLAGS) $(WARNINGS) -I. $< -lm $(LIBS) $(LINK_GL1) -lEGL -lwayland-egl -o $@$(EXT)
else ifeq ($(detected_OS),NetBSD)
	$(CC) $(CFLAGS) $(WARNINGS) $(CFLAGS) -I. $< -lm -o $@$(EXT)
else ifeq ($(detected_OS),Linux)
	$(CC) $(CFLAGS) $(WARNINGS)  -I. $< -lm  -o $@$(EXT)
else ifeq ($(detected_OS),windows)
	$(CC) $(CFLAGS) $(WARNINGS) -I. $< -lgdi32 -D UNICODE -o $@$(EXT)
else ifeq ($(detected_OS),Darwin)
	$(CC) $(CFLAGS) $(WARNINGS) -I. $< -framework CoreVideo -framework IOKit -framework Cocoa  -o $@$(EXT)
else
	$(CC) $(CFLAGS) $(WARNINGS) -I. $< $(LIBS) $(LINK_GL3) -o $@$(EXT)
endif

$(EXAMPLE_OUTPUTS): %: %.c RGFW.h $(WAYLAND_SOURCE)
	$(CC) $(CFLAGS) $(WARNINGS) -I. $< $(LINK_GL1) $(LIBS) $($)  -o $@$(EXT)

$(TEST_OUTPUTS): %: %.c RGFW.h $(WAYLAND_SOURCE)
	$(CC) $(CFLAGS) $(WARNINGS) -I. $< $(LINK_GL1) $(LIBS) $($)  -o $@$(EXT)
	@for exe in $(TEST_OUTPUTS); do \
		echo "Running $$exe..."; \
		./$$exe$(EXT); \
	done

debug: all
	@for exe in $(EXAMPLE_OUTPUTS); do \
		echo "Running $$exe..."; \
		./$$exe$(EXT); \
	done

	./examples/gamepad/gamepad
	./examples/first-person-camera/camera
	./examples/portableGL/pgl$(EXT)
	./examples/microui_demo/microui_demo
	./examples/gl33/gl33$(EXT)
ifneq ($(NO_GLES), 1)
		./examples/gles2/gles2$(EXT)
endif
ifneq ($(NO_OSMESA), 1)
		./examples/osmesa_demo/osmesa_demo$(EXT)
endif
ifneq ($(NO_VULKAN), 1)
		./examples/vk10/vk10$(EXT)
endif
ifeq ($(detected_OS), windows)
		./examples/dx11/dx11.exe
endif
	$(MAKE) clean


RGFW$(OBJ_FILE): RGFW.h $(WAYLAND_SOURCE)
	#$(CC) -x c $(CUSTOM_CFLAGS) -c RGFW.h -D RGFW_IMPLEMENTATION -fPIC -D RGFW_EXPORT
	cp RGFW.h RGFW.c
	$(CC) $(CUSTOM_CFLAGS) -c RGFW.c -D RGFW_IMPLEMENTATION -fPIC -D RGFW_EXPORT
	rm RGFW.c

libRGFW$(LIB_EXT): RGFW.h RGFW$(OBJ_FILE)
	$(MAKE) RGFW$(OBJ_FILE)
ifeq ($(CC), cl)
	link /DLL /OUT:libRGFW.dll RGFW.obj
else
	$(CC) $(CUSTOM_CFLAGS) -shared RGFW$(OBJ_FILE) $(LIBS) -o libRGFW$(LIB_EXT)
endif

libRGFW.a: RGFW.h RGFW$(OBJ_FILE)
	$(MAKE) RGFW$(OBJ_FILE)
	$(AR) rcs libRGFW.a RGFW$(OBJ_FILE)

ifeq ($(WAYLAND),1)

$(WAYLAND_SOURCE): %.c:
	$(MAKE) -f wayland.mk

endif


clean:
	rm -f *.o *.obj *.dll .dylib *.a *.so $(EXAMPLE_OUTPUTS) $(EXAMPLE_OUTPUTS_CUSTOM) $(TEST_OUTPUTS)  .$(OS_DIR)examples$(OS_DIR)*$(OS_DIR)*.exe .$(OS_DIR)examples$(OS_DIR)*$(OS_DIR)*.js .$(OS_DIR)examples$(OS_DIR)*$(OS_DIR)*.wasm .$(OS_DIR)examples$(OS_DIR)vk10$(OS_DIR)shaders$(OS_DIR)*.h

ifeq ($(WAYLAND),1)
	$(MAKE) -f wayland.mk clean
endif


.PHONY: all examples clean

