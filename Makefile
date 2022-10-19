# always use as many cores as possible
MAKEFLAGS = -j$(exec nproc)

# directories
BLD_DIR ?= build
SRC_DIR ?= src
BIN_DIR := $(BLD_DIR)/bin
OBJ_DIR := $(BLD_DIR)/obj
DEP_DIR := $(BLD_DIR)/dep

# flags and compiler
SHELL		= /bin/sh
CC			= gcc
LINKER		= $(CC)
INCLUDE		= -I$(SRC_DIR)
CPPFLAGS	=
CFLAGS		= -g -Wall -Wextra -std=c99 -ggdb3 -pedantic
LDFLAGS		= 
LDLIBS		= -lm -lSDL2

# directory tree
DIRS := $(BLD_DIR) $(BIN_DIR) $(OBJ_DIR) $(DEP_DIR) \
		$(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(shell find $(SRC_DIR) -type d -not -path $(SRC_DIR))) \
		$(patsubst $(SRC_DIR)/%,$(DEP_DIR)/%,$(shell find $(SRC_DIR) -type d -not -path $(SRC_DIR)))

# files
BIN := $(BIN_DIR)/main
SRC := $(shell find $(SRC_DIR) -type f -name '*.c')
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEP := $(SRC:$(SRC_DIR)/%.c=$(DEP_DIR)/%.d)

# echo output
RUN_CMD_CC		= @echo "  CC    " $@;
RUN_CMD_LTLINK	= @echo "  LTLINK" $@;

# build
all: $(DIRS) $(BIN)

# build and run
run: all
	@exec $(BIN)

# create directories
$(DIRS):
	@mkdir -p $@

# compile to binary
$(BIN): $(OBJ)
	$(RUN_CMD_LTLINK) $(LINKER) -o $@ $^ $(LDFLAGS) $(LDLIBS)

# generate object files and dependencies
$(OBJ): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(RUN_CMD_CC) $(CC) $(INCLUDE) $(CPPFLAGS) $(CFLAGS) -MMD -MP -MF $(<:$(SRC_DIR)/%.c=$(DEP_DIR)/%.d) -MT $@ -o $@ -c $<

# remove build files
clean:
	@rm -r $(BLD_DIR) 2> /dev/null || true

-include $(DEP)

.PHONY: all clean run
