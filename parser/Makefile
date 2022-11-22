EXTENSION?=txt
FORMAT?=DEFAULT
.PHONY: all parse clean raylib_api


raylib_parser: raylib_parser.c
	cc raylib_parser.c -o raylib_parser

raylib_api: ../src/raylib.h raylib_parser
	FORMAT=DEFAULT EXTENSION=txt $(MAKE) raylib_api.txt
	FORMAT=JSON EXTENSION=json $(MAKE) raylib_api.json
	FORMAT=XML EXTENSION=xml $(MAKE) raylib_api.xml
	FORMAT=LUA EXTENSION=lua $(MAKE) raylib_api.lua

raylib_api.$(EXTENSION): ../src/raylib.h raylib_parser
	./raylib_parser -i ../src/raylib.h -o raylib_api.$(EXTENSION) -f $(FORMAT) -d RLAPI

raymath_api.$(EXTENSION): ../src/raymath.h raylib_parser
	./raylib_parser -i ../src/raymath.h -o raymath_api.$(EXTENSION) -f $(FORMAT) -d RMAPI

rlgl_api.$(EXTENSION): ../src/rlgl.h raylib_parser
	./raylib_parser -i ../src/rlgl.h -o rlgl_api.$(EXTENSION) -f $(FORMAT) -d RLAPI -t "RLGL IMPLEMENTATION"

reasings_api.$(EXTENSION): ../examples/others/reasings.h raylib_parser
	./raylib_parser -i ../examples/others/reasings.h -o reasings_api.$(EXTENSION) -f $(FORMAT) -d EASEDEF

rmem_api.$(EXTENSION): ../rmem.h raylib_parser
	./raylib_parser -i ../rmem.h -o rmem_api.$(EXTENSION) -f $(FORMAT) -d RMEMAPI -t "RMEM IMPLEMENTATION"

physac_api.$(EXTENSION): ../physac.h raylib_parser
	./raylib_parser -i ../physac.h -o physac_api.$(EXTENSION) -f $(FORMAT) -d PHYSACDEF -t "PHYSAC IMPLEMENTATION"

raygui_api.$(EXTENSION): ../raygui.h raylib_parser
	./raylib_parser -i ../raygui.h -o raygui_api.$(EXTENSION) -f $(FORMAT) -d RAYGUIAPI -t "RAYGUI IMPLEMENTATION"

parse: raylib_api.$(EXTENSION) raymath_api.$(EXTENSION) rlgl_api.$(EXTENSION) rmem_api.$(EXTENSION) physac_api.$(EXTENSION) raygui_api.$(EXTENSION)


# `make parse` (and therefore `make all) requires
# rmem.h, physac.h and raygui.h to exist in the correct directory
# API files for individual headers can be created likeso, provided the relevant header exists:
#     FORMAT=JSON EXTENSION=json make raygui_api.json
all: raylib_parser
	FORMAT=DEFAULT EXTENSION=txt $(MAKE) parse
	FORMAT=JSON EXTENSION=json $(MAKE) parse
	FORMAT=XML EXTENSION=xml $(MAKE) parse
	FORMAT=LUA EXTENSION=lua $(MAKE) parse

clean:
	rm -f raylib_parser *.json *.txt *.xml *.lua
