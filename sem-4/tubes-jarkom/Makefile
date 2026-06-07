CC ?= clang
BUILD_DIR := build
SRC_DIR := src
INCLUDE_DIR := vendor
TARGET := $(BUILD_DIR)/magi

CSTD := -std=c2x
WARN := -Wall -Wextra -Wpedantic -Werror
DEFINES := -D_POSIX_C_SOURCE=200809L
INCLUDES := -Isrc -Ivendor
COMMON_FLAGS = $(CSTD) $(WARN) $(DEFINES) $(INCLUDES)
RELEASE_FLAGS := -O2 -DNDEBUG
DEBUG_FLAGS := -g -fsanitize=address,undefined -fno-omit-frame-pointer

SOURCES := $(shell find $(SRC_DIR) -name '*.c' | sort)
INCLUDE := $(shell find $(INCLUDE_DIR) -name '*.c' | sort)
ALL_SOURCES := $(SOURCES) $(INCLUDE)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))
INCLUDE_OBJECTS := $(patsubst $(INCLUDE_DIR)/%.c,$(BUILD_DIR)/$(INCLUDE_DIR)/%.o,$(INCLUDE))
ALL_OBJECTS := $(OBJECTS) $(INCLUDE_OBJECTS)
DEPFILES := $(OBJECTS:.o=.d) $(INCLUDE_OBJECTS:.o=.d)

TEST_DIR := tests
TEST_SOURCES := $(shell find $(TEST_DIR) -name 'test_*.c' 2>/dev/null | sort)
TEST_BINARIES := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/$(TEST_DIR)/%,$(TEST_SOURCES))

FORMAT_FILES := $(shell find $(SRC_DIR) $(INCLUDE_DIR) -name '*.c' -o -name '*.h' | sort)

MODE ?= release

MODE_FLAGS = $(if $(filter debug,$(MODE)),$(DEBUG_FLAGS),$(RELEASE_FLAGS))

all: compile

compile: $(TARGET)

run: MODE=release
run:
	$(MAKE) MODE=$(MODE) clean compile
	@$(TARGET)

debug: MODE=debug
debug:
	$(MAKE) MODE=$(MODE) clean compile

debug-run: debug
	@$(TARGET)

async: DEFINES += -DMAGI_ASYNC
async:
	$(MAKE) MODE=$(MODE) DEFINES="$(DEFINES)" clean compile

async-run: async
	@$(TARGET)

gui:
	$(MAKE) MODE=$(MODE) clean compile

$(TARGET): $(ALL_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(MODE_FLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(MODE_FLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR)/$(INCLUDE_DIR)/%.o: $(INCLUDE_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(MODE_FLAGS) -MMD -MP -c $< -o $@

-include $(DEPFILES)

clean:
	rm -rf $(BUILD_DIR)

format:
	clang-format -i $(FORMAT_FILES)

test: $(TEST_BINARIES)
	@for test in $(TEST_BINARIES); do \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done

$(BUILD_DIR)/$(TEST_DIR)/%: $(TEST_DIR)/%.c $(filter-out $(BUILD_DIR)/main.o, $(ALL_OBJECTS))
	@mkdir -p $(dir $@)
	$(CC) $(COMMON_FLAGS) $(MODE_FLAGS) $^ -o $@

.PHONY: all compile run debug async gui clean test