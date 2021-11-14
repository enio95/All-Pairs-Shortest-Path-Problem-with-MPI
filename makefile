 # source directory
SRC_DIR := src
 # Object directory (*.o)
OBJ_DIR := obj
 # Executables directory
BIN_DIR := bin
 # Headers file directory
INCLUDE_DIR := include
# Executable path name
EXE := $(BIN_DIR)/fox

# Compiler program
CC := mpicc
# Include directories in search path
CFLAGS := -I$(INCLUDE_DIR)
LFLAGS := -lm

# How to run
RP := mpirun
RFLAGS := --oversubscribe -np 9

SRC := $(wildcard $(SRC_DIR)/*.c)
DEP := $(wildcard $(INCLUDE_DIR)/*.h)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all compile clean run

 # Compile and run
all: compile run

run:
	$(RP) $(RFLAGS) ./$(EXE) 

compile: $(EXE)

# Link the object files into a executable. Also checks if 
# the object directory exists, if not creates it
$(EXE): $(OBJ) | $(OBJ_DIR)
	$(CC) $^ -o $@ $(LFLAGS)

# Generate the object file without linking 
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEP)
	$(CC) $(CFLAGS) -c $< -o $@

# Create bin and obj directory in case they dont exist
$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

clean:
	@rm -vf $(BIN_DIR)/*
	@rm -vf $(OBJ_DIR)/*.o