#tool names
CC = gcc
STRIP = strip

#binary name
BIN = rogal
#directory to put the build in
BUILD_DIR = ./bin/Linux
#obj directory
OBJ_DIR = $(BUILD_DIR)/obj
#output directory
OUT_DIR = $(BUILD_DIR)/out
#include files directory
IDIR = ./rogal/headers
#resources directory
RES_DIR = ./rogal/resources

#compiler flags
CFLAGS = -Wall -Wpedantic -Wextra -I$(IDIR)
LIBS = -lglut -lm -lGL -lGLU

#add debug flag
CFLAGS += -D _DEBUG

#console output control
EXEC = @
ECHO = @echo

#files to compile
CFILES = $(wildcard rogal/source/*.c)         \
		 $(wildcard rogal/source/game/*.c)    \
		 $(wildcard rogal/source/physics/*.c) \
		 $(wildcard rogal/source/render/*.c)  \
		 $(wildcard rogal/source/ui/*.c)      \

#object files created from cfiles
OBJ = $(CFILES:%.c=$(OBJ_DIR)/%.o)

#dependencies from objects
DEPS = $(OBJ:%.o=%.d)

#default target
$(BIN): $(OUT_DIR)/$(BIN)

#main target
$(OUT_DIR)/$(BIN): $(OBJ)
	$(ECHO) [LINK ] $@
	$(EXEC) mkdir -p $(@D)
	$(EXEC) $(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	$(ECHO) [COPY ]
	$(EXEC) cp -r $(RES_DIR) $(OUT_DIR)/resources
	
#include dependencies
-include $(DEPS)

#build every c file
$(OBJ_DIR)/%.o: %.c
	$(ECHO) [BUILD] $@
	$(EXEC) mkdir -p $(@D)
	$(EXEC) $(CC) $(CFLAGS) -MMD -c $< -o $@ $(LIBS)
	
#command targets
.PHONY: clean strip

#clean files created by make
clean:
	$(ECHO) [CLEAN]
	$(EXEC) rm -f $(OUT_DIR)/$(BIN) $(OBJ) $(DEPS)
	$(EXEC) rm -rf $(OBJ_DIR) $(OUT_DIR)
	
#strip debugging symbols from the compiled file
strip: $(BIN)
	$(ECHO) [STRIP] $^
	$(EXEC) $(STRIP) $(OUT_DIR)/$(BIN)
