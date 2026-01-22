# This is a makefile for compiling your whole project.
# It is used to compile all the source files in the src directory and link them together.
# It is also used to clean up the build directory and remove all the object files and the executable.
# The makefile is used to compile the project using the gcc compiler.

# Things to explore:
# - Linux, MacOS, WSL for Windows
# - Makefile variables
# - Makefile rules
# - Makefile targets
# - Makefile dependencies
# - Makefile commands

# NOTE: YOU MAY CHANGE THIS MAKEFILE TO YOUR NEEDS
TARGET      := bin/main
SRC_DIR     := src
BUILD_DIR   := build

SRC_FILES   := $(shell find $(SRC_DIR) -name "*.c" ! -name "*_driver.c")
OBJ_FILES   := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Compiler and Flags
CC          := gcc
CFLAGS      := -Wall -Wextra -I$(SRC_DIR)
DEBUG_FLAGS := -g

# Define phony targets
.PHONY: all debug run clean clean-bin clean-all

# Build the program
all: $(TARGET)

# link the final executable
$(TARGET): $(OBJ_FILES)
	@mkdir -p $(dir $@)
	$(CC) $(OBJ_FILES) -o $@

# compile each .c into .o under build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

# Debug build
debug: CFLAGS += -DDEBUG
debug: all

# Run the program
run: all
	@./$(TARGET)

# Clean build files
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR) $(TARGET)

# Clean binary file
clean-bin:
	@echo "Removing bin directory..."
	rm -rf bin

clean-all: clean clean-bin
