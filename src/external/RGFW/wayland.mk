SYSTEM_WAYLAND := /usr/share/wayland-protocols
LOCAL_WAYLAND  := ./deps/wayland

WAYLAND_PROTO := \
	stable/xdg-shell/xdg-shell \
	unstable/xdg-decoration/xdg-decoration-unstable-v1 \
	staging/xdg-toplevel-icon/xdg-toplevel-icon-v1 \
	unstable/relative-pointer/relative-pointer-unstable-v1 \
	unstable/pointer-constraints/pointer-constraints-unstable-v1 \
	unstable/xdg-output/xdg-output-unstable-v1 \
	staging/pointer-warp/pointer-warp-v1 \


WAYLAND_HEADERS = $(addsuffix .h,$(WAYLAND_PROTO))
WAYLAND_SOURCE = $(addsuffix .c,$(WAYLAND_PROTO))

# if the file exists in $(SYSTEM_WAYLAND) exists, use it else use the one in $(LOCAL_WAYLAND) but without the extra path
define find_xml
$(if $(wildcard $(SYSTEM_WAYLAND)/$(1)),\
     $(SYSTEM_WAYLAND)/$(1),\
     $(LOCAL_WAYLAND)/$(notdir $(1)))
endef

all: $(WAYLAND_HEADERS) $(WAYLAND_SOURCE)

$(WAYLAND_HEADERS): %.h:
	wayland-scanner client-header $(call find_xml,$(@:.h=.xml)) $(notdir $@)

$(WAYLAND_SOURCE): %.c:
	wayland-scanner public-code $(call find_xml,$(@:.c=.xml)) $(notdir $@)

sources:
	@echo $(notdir $(WAYLAND_SOURCE))

clean:
	rm $(notdir $(WAYLAND_SOURCE)) $(notdir $(WAYLAND_HEADERS))
