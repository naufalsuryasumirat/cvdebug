# Dependencies:
# 1. SDL2 (Linux/Windows)
# 2. OpenGL3
# 3. OpenCV 4.10.0 (Build it yourself)
# 4. fmt
# 5(opt). zig

# this makefile is not cross-platform guaranteed, tested on Ubuntu 24.04.1 LTS and zig-nightly 14

CC = zig cc
CXX = zig c++

TARGET_DIR = ./bin
TARGET = $(TARGET_DIR)/trackbar_main 

OBJ_DIR = ./obj
TRACKBAR_DIR = ./trackbar
IMGUI_DIR = $(TRACKBAR_DIR)/imgui

CCJ = compile_commands.json

# linker opencv
# TODO: link opencv
LIB_PATH = 

SOURCES = main.cpp
SOURCES += $(TRACKBAR_DIR)/trackbar.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
OBJS = $(addprefix $(OBJ_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL

CXXFLAGS = -MJ $(CCJ) -std=c++11 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends
CXXFLAGS += -g -Wall -Wformat
LIBS =

# assumes OpenGL3 is available system-wide
ifeq ($(UNAME_S), Linux)
	ECHO_MESSAGE = "Linux"
	LIBS += $(LINUX_GL_LIBS) -ldl `sdl2-config --libs`

	CXXFLAGS += `sdl2-config --cflags`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
    ECHO_MESSAGE = "MinGW"
    LIBS += -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`

    CXXFLAGS += `pkg-config --cflags sdl2`
    CFLAGS = $(CXXFLAGS)
endif

# build rules
$(OBJ_DIR)/%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o:$(TRACKBAR_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(TARGET)
	@echo Build complete for $(ECHO_MESSAGE)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(TARGET) $(OBJS) $(CCJ)

# TODO: write other flags (debug, release w/ debug, release w/o debug)
