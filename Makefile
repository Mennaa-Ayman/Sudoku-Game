# Project settings
INCLUDE_DIR = include
SOURCE_DIR  = src
BUILD_DIR   = build

# Qt settings (override QT_ROOT or rely on QTDIR from the Qt environment)
QT_ROOT ?= $(QTDIR)
QT_INCLUDE_DIR = $(QT_ROOT)/include
QT_LIB_DIR     = $(QT_ROOT)/lib
QT_BIN_DIR     = $(QT_ROOT)/bin
MOC            = "$(QT_BIN_DIR)/moc"

# Tools and flags
CXX       = g++
CPPFLAGS  = -I$(INCLUDE_DIR)
CXXFLAGS  = -std=c++17 -Wall -Wextra -O2
LDFLAGS   =
LDLIBS    =

ifeq (,$(QT_ROOT))
$(error QT_ROOT is not set. Set QT_ROOT (or QTDIR) to your Qt installation root before building.)
else
CPPFLAGS += -I"$(QT_INCLUDE_DIR)" -I"$(QT_INCLUDE_DIR)/QtWidgets" -I"$(QT_INCLUDE_DIR)/QtGui" -I"$(QT_INCLUDE_DIR)/QtCore"
LDFLAGS  += -L"$(QT_LIB_DIR)"
LDLIBS   += -lQt6Widgets -lQt6Gui -lQt6Core
endif

# Source files
SRCS := $(wildcard $(SOURCE_DIR)/*.cpp)

# Meta-object compiler sources
MOC_HEADERS := $(INCLUDE_DIR)/UserInterface.hpp
MOC_SRCS    := $(patsubst $(INCLUDE_DIR)/%.hpp,$(BUILD_DIR)/moc_%.cpp,$(MOC_HEADERS))
SRCS        += $(MOC_SRCS)

# Object files
OBJS := $(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(wildcard $(SOURCE_DIR)/*.cpp))
OBJS += $(patsubst $(BUILD_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(MOC_SRCS))

# Target executable
TARGET = Sudoku.exe

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	@echo Linking...
	$(CXX) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@
	@echo Build complete: $(TARGET)

# Compile .cpp -> .o
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(BUILD_DIR)
	@echo Compiling $< ...
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.cpp | $(BUILD_DIR)
	@echo Compiling $< ...
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/moc_%.cpp: $(INCLUDE_DIR)/%.hpp | $(BUILD_DIR)
	@echo Generating MOC for $< ...
	$(MOC) $(CPPFLAGS) $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir $(BUILD_DIR)

# Clean
clean:
	@echo Cleaning...
	@if exist "$(BUILD_DIR)" rmdir /S /Q $(BUILD_DIR)
	@if exist "$(TARGET)" del /Q $(TARGET)
	@echo Done.

# Run the program
run: all
	@echo Running $(TARGET)...
	@set PATH=$(QT_BIN_DIR);%PATH% && .\$(TARGET)

# Force rebuild
rebuild: clean all

.PHONY: all clean rebuild run
