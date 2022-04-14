EXTENSION?=txt
FORMAT?=DEFAULT

raylib_api:
	cc raylib_parser.c -o raylib_parser
	./raylib_parser -i ../src/raylib.h -o raylib_api.txt -f DEFAULT -d RLAPI
	./raylib_parser -i ../src/raylib.h -o raylib_api.json -f JSON -d RLAPI
	./raylib_parser -i ../src/raylib.h -o raylib_api.xml -f XML -d RLAPI
	./raylib_parser -i ../src/raylib.h -o raylib_api.lua -f LUA -d RLAPI

all:
	cc raylib_parser.c -o raylib_parser
	FORMAT=DEFAULT EXTENSION=txt $(MAKE) parse
	FORMAT=JSON EXTENSION=json $(MAKE) parse
	FORMAT=XML EXTENSION=xml $(MAKE) parse
	FORMAT=LUA EXTENSION=lua $(MAKE) parse

parse:
	./raylib_parser -i ../src/raylib.h -o raylib_api.$(EXTENSION) -f $(FORMAT) -d RLAPI
	./raylib_parser -i ../src/raymath.h -o raymath_api.$(EXTENSION) -f $(FORMAT) -d RMAPI
	./raylib_parser -i ../src/extras/easings.h -o easings_api.$(EXTENSION) -f $(FORMAT) -d EASEDEF
	./raylib_parser -i ../src/extras/physac.h -o physac_api.$(EXTENSION) -f $(FORMAT) -d PHYSACDEF
	./raylib_parser -i ../src/extras/raygui.h -o raygui_api.$(EXTENSION) -f $(FORMAT) -d RAYGUIAPI
	./raylib_parser -i ../src/extras/rmem.h -o rmem_api.$(EXTENSION) -f $(FORMAT) -d RMEMAPI
	./raylib_parser -i ../src/rlgl.h -o rlgl_api.$(EXTENSION) -f $(FORMAT) -d RLAPI

clean:
	rm -f raylib_parser *.json *.txt *.xml *.lua
