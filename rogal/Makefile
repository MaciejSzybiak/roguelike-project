#tool names
CC = gcc
STRIP = strip

#binary name
BIN = rogal
#directory to put the build in
BUILD_DIR = ./linux_build
#include files directory
IDIR = ./headers

#compiler flags
CFLAGS = -Wall -Wpedantic -Wextra -I$(IDIR)
LIBS = -lglut -lm -lGL -lGLU

#add debug flag
CFLAGS += -D _DEBUG

#console output control
EXEC = @
ECHO = @echo

#files to compile
CFILES = $(wildcard source/*.c) 		\
		 $(wildcard source/game/*.c) 	\
		 $(wildcard source/physics/*.c) \
		 $(wildcard source/render/*.c) 	\
		 $(wildcard source/ui/*.c)		\

#object files created from cfiles
OBJ = $(CFILES:%.c=$(BUILD_DIR)/%.o)

#dependencies from objects
DEPS = $(OBJ:%.o=%.d)

#default target
$(BIN): $(BUILD_DIR)/$(BIN)

#main target
$(BUILD_DIR)/$(BIN): $(OBJ)
	$(ECHO) [LINK ] $@
	$(EXEC) mkdir -p $(@D)
	$(EXEC) $(CC) $(CFLAGS) $^ -o $@ $(LIBS)
	
#include dependencies
-include $(DEPS)

#build every c file
$(BUILD_DIR)/%.o: %.c
	$(ECHO) [BUILD] $@
	$(EXEC) mkdir -p $(@D)
	$(EXEC) $(CC) $(CFLAGS) -MMD -c $< -o $@ $(LIBS)
	
#command targets
.PHONY: clean strip

#clean files created by make
clean:
	$(ECHO) [CLEAN]
	$(EXEC) rm -f $(BUILD_DIR)/$(BIN) $(OBJ) $(DEPS)
	
#strip debugging symbols from the compiled file
strip: $(BIN)
	$(ECHO) [STRIP] $^
	$(EXEC) $(STRIP) $(BUILD_DIR)/$(BIN)