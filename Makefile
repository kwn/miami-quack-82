# Makefile for ace-scroller
# Usage: export PATH=... && make
# (the PATH export is already done by the amiga-debug extension launch, or
#  you can run:  export PATH=$PATH:~/.vscode/extensions/bartmanabyss.amiga-debug-1.7.9/bin/darwin/opt/bin:~/.vscode/extensions/bartmanabyss.amiga-debug-1.7.9/bin/darwin && make)

BARTMAN_BIN  := $(HOME)/.vscode/extensions/bartmanabyss.amiga-debug-1.7.9/bin/darwin
TOOLCHAIN_BIN := $(BARTMAN_BIN)/opt/bin

TOOLCHAIN_CMAKE := $(CURDIR)/toolchain/m68k-bartman.cmake
BUILD_DIR       := build
GAME_EXE        := $(BUILD_DIR)/ace_scroller.exe

# Prepend toolchain to PATH so cmake/make can find the cross-compiler
export PATH := $(TOOLCHAIN_BIN):$(BARTMAN_BIN):$(PATH)

.PHONY: all clean configure

all: $(GAME_EXE)

# Rebuild when any source or cmake file changes
$(GAME_EXE): $(BUILD_DIR)/Makefile $(wildcard src/*.c) CMakeLists.txt
	$(MAKE) -C $(BUILD_DIR)

# Configure step: run cmake once to generate the inner Makefile
$(BUILD_DIR)/Makefile: CMakeLists.txt toolchain/m68k-bartman.cmake
	mkdir -p $(BUILD_DIR)
	cmake -B $(BUILD_DIR) -S . \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN_CMAKE) \
		-DCMAKE_BUILD_TYPE=Debug

configure: $(BUILD_DIR)/Makefile

clean:
	rm -rf $(BUILD_DIR)
