# Makefile – TinyShell (MinGW / Windows)
# Chay: mingw32-make   hoac   make

CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g -Iinclude
LDFLAGS  = -lkernel32
TARGET   = bin/myShell.exe

# Giai doan Duy: Core + Parser + stub tam cho cac module chua co.
# Khi Huy / Hoang / Duong xong, bo stubs.cpp va them file .cpp tuong ung.
SRCS = src/main.cpp \
       src/shell.cpp \
       src/parser.cpp \
       src/stubs.cpp

OBJS = $(SRCS:src/%.cpp=build/%.o)

all: $(TARGET)

$(TARGET): $(OBJS) | bin
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c -o $@ $<

bin:
	if not exist bin mkdir bin

build:
	if not exist build mkdir build

clean:
	if exist build rmdir /S /Q build
	if exist "bin\myShell.exe" del /Q "bin\myShell.exe"

.PHONY: all clean
