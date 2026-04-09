# A makefile for linux to compile to three targets: linux, windows, and web (Make sure to setup the libraries in the include directory first)
# Big thanks to Gemini 3 Pro as the sponsor of todays file.

# --- Variables ---
CXX = clang++
CXXFLAGS = -std=c++20 -Iinclude -O3 -flto -Werror -march=x86-64-v3 -pthread -static
WIN_TARGET = --target=x86_64-w64-mingw32 -fuse-ld=lld

# --- Targets ---

all: linux windows web

linux:
	@echo "Compiling for Linux..."
	$(CXX) $(CXXFLAGS) $(LINUX_STATIC) wggcalc-cli.cpp -o wggcalc

windows:
	@echo "Compiling for Windows..."
	$(CXX) $(WIN_TARGET) $(CXXFLAGS) wggcalc-cli.cpp -o wggcalc.exe

web:
	@echo "Compiling for Web..."
	em++ -std=c++20 -Iinclude wggcalc-wasm.cpp -o Web/src/libs/wggcalc.js \
	--bind --no-entry -s MODULARIZE=1 -s EXPORT_NAME='createModule' \
	-s EXPORT_ES6=1 --emit-tsd wggcalc.d.ts -O3 -flto -Werror -std=c++20 \

clean:
	@echo "Cleaning up..."
	rm -f wggcalc wggcalc.exe Web/src/libs/wggcalc.js Web/src/libs/wggcalc.wasm Web/src/libs/wggcalc.d.ts