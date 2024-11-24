EXAMPLE = core/core_input_gamepad
PLATFORM = PLATOFRM_WEB_RGFW  OS=linux


all:
	rm -f  examples/$(EXAMPLE).exe examples/$(EXAMPLE).html examples/$(EXAMPLE).wasm examples/$(EXAMPLE).js  src/rcore.o
	cd src && make PLATFORM=$(PLATFORM)	
	cd examples && make $(EXAMPLE) PLATFORM=$(PLATFORM)
