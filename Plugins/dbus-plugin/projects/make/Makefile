# This is a rudimentary Makefile that compiles files from the pluginsdk & src directories into a
# shared library. Adapt to your needs.

# Rename to your plugin's name.
TARGET = DbusPlugin

CC = gcc
CXX = g++

CPPFLAGS += -Wall -Wextra -g -pipe -O3 -fPIC -I../../
CXXFLAGS += -std=c++0x
LINKFLAGS += -g -O3 -shared -Wall
LINKXXFLAGS += `pkg-config --libs gtk+-3.0`

CPPFLAGS += `pkg-config --cflags gtk+-3.0`

VPATH = ../../

OUTPUT_DIR = build

OBJS = \
	pluginsdk/Config.o \
	pluginsdk/Connections.o \
	pluginsdk/Core.o \
	pluginsdk/Hooks.o \
	pluginsdk/Hubs.o \
	pluginsdk/Logger.o \
	pluginsdk/Queue.o \
	pluginsdk/Tagger.o \
	pluginsdk/UI.o \
	pluginsdk/Util.o \
	src/main.o \
	src/Plugin.o \
	src/stdafx.o

COMPILER_SPEC = $(shell $(CC) -dumpmachine)

ifneq ($(findstring mingw, $(COMPILER_SPEC)),)
	WIN_TARGET = 1
endif
ifeq ($(OS), Windows_NT)
	ifeq ($(findstring Cygwin, $(SHELL)),)
		WIN_SHELL = 1
	endif
endif

ifdef WIN_TARGET
	BINEXT = .exe
	CPPFLAGS += -D_WIN32_WINNT=0x502 -DWINVER=0x502 -D_WIN32_IE=0x600 \
				-DNOMINMAX -DSTRICT -DWIN32_LEAN_AND_MEAN \
				-DWIN32 -D_WIN32 -DUNICODE -D_UNICODE
	LIBEXT = .dll
	OBJS += src/resource.o
	OUTPUT_DIR := $(OUTPUT_DIR)-mingw
else
	BINEXT =
	LIBEXT = .so
endif

ifeq ($(findstring x86_64, $(COMPILER_SPEC)),)
	CPPFLAGS += -march=i686
	OUTPUT_DIR := $(OUTPUT_DIR)-x86
	TARGET := $(TARGET)-x86
else
	OUTPUT_DIR := $(OUTPUT_DIR)-x64
	TARGET := $(TARGET)-x64
endif

ifdef WIN_SHELL
	MKDIR = if not exist $(subst /,\,$1) md $(subst /,\,$1)
	RMDIR = if exist $(subst /,\,$1) rd /s /q $(subst /,\,$1)
else
	MKDIR = mkdir -p $1
	RMDIR = $(RM) -r $1
endif

ifdef WIN_TARGET
	PDB := $(OUTPUT_DIR)/$(TARGET).pdb

	ifdef WIN_SHELL
		RCFLAGS := -DPLUGIN_FILE_NAME="^\\"$(TARGET)\\"" \
			-DPLUGIN_FILE_NAME_EXT="^\\"$(TARGET)$(LIBEXT)\\""
	else
		RCFLAGS := -DPLUGIN_FILE_NAME="\\\"$(TARGET)\\\"" \
			-DPLUGIN_FILE_NAME_EXT="\\\"$(TARGET)$(LIBEXT)\\\""
	endif
else
	PDB =
endif

TARGET := $(OUTPUT_DIR)/$(TARGET)$(LIBEXT)
OBJS := $(addprefix $(OUTPUT_DIR)/, $(OBJS))

all: $(TARGET) $(PDB) $(PACK_INFO)

$(TARGET): $(OBJS)
	$(CXX) $^ $(LINKFLAGS) $(LINKXXFLAGS) $(OUTPUT_OPTION)

ifdef WIN_TARGET
$(PDB): $(TARGET)
	strip --only-keep-debug $< $(OUTPUT_OPTION) && strip $<
endif

$(PACK_INFO): $(PACKAGER)
	$(PACKAGER) $(PACK_INFO)

$(OUTPUT_DIR)/%.o: %.c
	$(COMPILE.c) $< $(OUTPUT_OPTION)

$(OUTPUT_DIR)/%.o: %.cpp
	$(COMPILE.cpp) $< $(OUTPUT_OPTION)

ifdef WIN_TARGET
$(OUTPUT_DIR)/%.o: %.rc
	windres $< $(RCFLAGS) $(OUTPUT_OPTION)
endif

$(OBJS): | $(OUTPUT_DIR)

#$(PACKAGER): | $(OUTPUT_DIR)

$(OUTPUT_DIR):
	#$(call MKDIR,$(OUTPUT_DIR)/packaging/packager)
	$(call MKDIR,$(OUTPUT_DIR)/pluginsdk)
	$(call MKDIR,$(OUTPUT_DIR)/src)

clean:
	$(call RMDIR,$(OUTPUT_DIR))
