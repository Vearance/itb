# Makefile for C++ OOP Project (Optimized & Recursive)

# Compiler settings
CXX      := g++
CLANG_FORMAT := $(shell command -v clang-format || command -v clang-format-18 || command -v clang-format-17 || command -v clang-format-16 || command -v clang-format-15 || command -v clang-format-14)

# Directories
SRC_DIR     := src
OBJ_DIR     := build
BIN_DIR     := bin
INCLUDE_DIR := include
DATA_DIR    := data
CONFIG_DIR  := config

DOC_DIR     := doc

# Header search paths
INCLUDE_DIRS := $(shell find $(INCLUDE_DIR) -type d)
CPPFLAGS := $(addprefix -I,$(INCLUDE_DIRS))
CXXFLAGS := -Wall -Wextra -std=c++17

# Target executable
TARGET := $(BIN_DIR)/game

# 1. Recursive Source Finding
# Secara otomatis mencari semua file .cpp di dalam src/ dan semua sub-foldernya
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
FORMAT_FILES := $(shell find $(SRC_DIR) $(INCLUDE_DIR) -type f \( -name '*.cpp' -o -name '*.cc' -o -name '*.cxx' -o -name '*.hpp' -o -name '*.h' \))

# 2. Dynamic Object Mapping
# Mengubah path src/xxx/yyy.cpp menjadi build/xxx/yyy.o
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Main targets
all: directories $(TARGET)

# Create necessary root directories
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR) $(DATA_DIR) $(CONFIG_DIR) $(DOC_DIR)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@
	@echo "Build successful! Executable is at $(TARGET)"

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Run the game
run: all
	./$(TARGET)

# Clean up generated files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned up $(OBJ_DIR) and $(BIN_DIR)"

# Rebuild everything from scratch
rebuild: clean all

check-clang-format:
	@[ -n "$(CLANG_FORMAT)" ] || { echo "Error: clang-format is not installed."; exit 1; }

format: check-clang-format
	$(CLANG_FORMAT) -i $(FORMAT_FILES)

check-format: check-clang-format
	@$(CLANG_FORMAT) -n --Werror $(FORMAT_FILES)

diagram: directories
	@doxygen Doxyfile
	@hpp2plantuml -i "include/**/*.hpp" -t $(DOC_DIR)/template.puml -o $(DOC_DIR)/class_diagrams.puml
	@python3 scripts/generate_class_diagrams.py $(DOC_DIR)/class_diagrams.puml

.PHONY: all clean rebuild run directories check-clang-format format check-format