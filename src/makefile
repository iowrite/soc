.PHONY: clean

# Variables
SRC_DIR := ../src
BUILD_DIR := build_dir
TARGET := build_dir/libmyekfsoc.a

# Source files and their corresponding object files
SRCS := port debug soc curve curve_314_ruipu soh soe sop sox common
OBJS := $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(SRCS)))
DEPS := $(OBJS:.o=.d)

# Compiler flags
CLIBFLAGS := -xc -std=c99 -Wall -Wextra -Wconversion -Wstrict-prototypes -ffunction-sections -fdata-sections -O0 -g -fPIC




# Default target
all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Archive target
$(TARGET): $(OBJS)
	llvm-ar rcs $@ $^

# Pattern rule for object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	clang -DSOX_USER_CONFIG_FILE=$(USER_CONFIG) $(CLIBFLAGS) -c $< -o $@

# Pattern rule for dependency files
$(BUILD_DIR)/%.d: $(SRC_DIR)/%.c | $(BUILD_DIR)
	clang -DSOX_USER_CONFIG_FILE=$(USER_CONFIG) -M $< -MF $@ -MT $(@:.d=.o)

# Include dependencies
-include $(DEPS)

# Clean target
clean:
	rm -rf $(BUILD_DIR)