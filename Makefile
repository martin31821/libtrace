target ?= release

PKGS = glib-2.0

override CFLAGS += `pkg-config --cflags $(PKGS)` -std=c99 -Wall -Wextra -I./include -pipe -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -fPIC
override LDFLAGS += `pkg-config --libs $(PKGS)`
override DBGFLAGS += -tui
BUILD_FOLDER = build/$(target)/
DIST_FOLDER = dist/
LIBRARY = trace

ifeq ($(target),debug)
override CFLAGS += -g3 -Og
else ifeq ($(target),release)
override CFLAGS += -O3
else
$(error Bad target name '$(target)'! Please choose 'debug' or 'release' only)
endif

LIB_SRCS = $(wildcard src/*.c) $(wildcard src/*/*.c) $(wildcard src/*/*/*.c)
LIB_OBJS = $(addprefix $(BUILD_FOLDER),$(LIB_SRCS:%.c=%.o))
LIB_DEPS = $(addprefix $(BUILD_FOLDER),$(LIB_SRCS:%.c=%.dep))

LIB = $(addprefix $(BUILD_FOLDER),$(LIBRARY))

all: __init__ $(LIB).dll
	@echo ">>> Build successful"

__init__:
	@echo ">>> Build target is '$(target)'"

clean: __init__
	@echo ">>> Cleaning..."
	@rm -f $(LIB).dll $(LIB_OBJS) $(LIB_DEPS) $(DIST_FOLDER)*.zip
	@find $(BUILD_FOLDER).. -type d -empty -delete 2>/dev/null || true
	@echo ">>> Clean successful"

lib: $(LIB).dll
	@echo ">>> Library built successfully"

dist: __init__ lib
	@echo "<<< Bundling all DLL's into zip..."
	@mkdir -p $(DIST_FOLDER)
	@ldd $(LIB).dll | \
		grep -vi /c/windows | \
		grep -oP "\s+.+? => \K.+\.dll" | \
		xargs zip -j $(DIST_FOLDER)$(target).zip $(LIB).dll
	@echo "<<< Finished bundling!"

$(LIB).dll: $(LIB_OBJS)
	@echo ">>> Linking library '$(LIBRARY).dll'..."
	@$(CC) -o $@ $^ $(CFLAGS) -L$(BUILD_FOLDER) $(LDFLAGS) -shared

$(BUILD_FOLDER)%.o: %.c
	@echo ">>> Compiling '$<'..."
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD_FOLDER)%.dep: %.c
	@echo "--- Collecting dependencies for '$<'..."
	@mkdir -p $(@D)
	@$(CC) -MM $(CFLAGS) -MT $(@:%.dep=%.o) $< > $@

lib-linux: $(LIB).a

$(LIB).a: $(LIB_OBJS)
	@echo ">>> Linking library '$(LIBRARY).a'..."
	@$(AR) rcs $@ $^
	
ifneq ($(MAKECMDGOALS),clean)
include $(LIB_DEPS) $(DEPS)
endif
