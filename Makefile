NAME = bard

# Compiler and flags
CC = clang
CFLAGS = -Wall -Wextra -std=c11 -Iinclude/

# LLVM paths
LLVM_CONFIG = llvm-config
LLVM_CFLAGS = $(shell $(LLVM_CONFIG) --cflags)
LLVM_LIBS = $(shell $(LLVM_CONFIG) --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter --system-libs)

# Additional libraries
LIBS = -lffi -ledit -lz -ltinfo $(LLVM_LIBS)

# Directories
OBJDIR = target/obj
BINDIR = target/bin

# Source files
SRC = src/bard.c
OBJ_DEBUG = $(SRC:src/%.c=$(OBJDIR)/debug/%.o)
OBJ_RELEASE = $(SRC:src/%.c=$(OBJDIR)/release/%.o)

# Targets
TARGET_DEBUG = $(BINDIR)/debug/$(NAME)
TARGET_RELEASE = $(BINDIR)/release/$(NAME)

# Rules
all: debug release

debug: CFLAGS += -g
debug: $(TARGET_DEBUG)

release: CFLAGS += -O2
release: LIBS += -s
release: $(TARGET_RELEASE)

$(TARGET_DEBUG): $(OBJ_DEBUG)
	@mkdir -p $(BINDIR)/debug
	$(CC) $(OBJ_DEBUG) -o $@ $(LIBS)

$(TARGET_RELEASE): $(OBJ_RELEASE)
	@mkdir -p $(BINDIR)/release
	$(CC) $(OBJ_RELEASE) -o $@ $(LIBS)

$(OBJDIR)/debug/%.o: src/%.c
	@mkdir -p $(OBJDIR)/debug
	$(CC) $(CFLAGS) $(LLVM_CFLAGS) -c $< -o $@

$(OBJDIR)/release/%.o: src/%.c
	@mkdir -p $(OBJDIR)/release
	$(CC) $(CFLAGS) $(LLVM_CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(BINDIR)
