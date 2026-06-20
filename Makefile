# Makefile – TinyShell (MinGW / Windows)
# Chay: mingw32-make   hoac   make

CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g -Iinclude
# Use -lkernel32 only on Windows
ifeq ($(OS),Windows_NT)
    LDFLAGS  = -lkernel32
    TARGET   = bin/myShell.exe
else
    LDFLAGS  =
    TARGET   = bin/myShell
endif

# Giai doan Duy: Core + Parser + stub tam cho cac module chua co.
# Khi Huy / Hoang / Duong xong, bo stubs.cpp va them file .cpp tuong ung.
SRCS = src/main.cpp \
       src/shell.cpp \
       src/parser.cpp \
       src/builtins.cpp \
       src/stubs.cpp

OBJS = $(SRCS:src/%.cpp=build/%.o)

all: $(TARGET)

$(TARGET): $(OBJS) | bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c -o $@ $<

bin:
ifeq ($(OS),Windows_NT)
	if not exist bin mkdir bin
else
	mkdir -p bin
endif

build:
ifeq ($(OS),Windows_NT)
	if not exist build mkdir build
else
	mkdir -p build
endif

clean:
ifeq ($(OS),Windows_NT)
	if exist build rmdir /S /Q build
	if exist "bin\myShell.exe" del /Q "bin\myShell.exe"
else
	rm -rf build
	rm -f bin/myShell
endif

.PHONY: all clean
