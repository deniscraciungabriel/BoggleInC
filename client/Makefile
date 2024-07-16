# Directories
SOURCE_DIRECTORY = src
HEADERS_DIRECTORY = headers
OBJECTS_DIRECTORY = objects
EXECUTABLES_DIRECTORY = executables

# Compiler settings
COMPILER = gcc
COMP_FLAGS = -I$(HEADERS_DIRECTORY) -Wall -Wextra -g

# Files and targets
SOURCE_FILES = $(wildcard $(SOURCE_DIRECTORY)/*.c)
HEADER_FILES = $(wildcard $(HEADERS_DIRECTORY)/*.h)
OBJECT_FILES = $(patsubst $(SOURCE_DIRECTORY)/%.c, $(OBJECTS_DIRECTORY)/%.o, $(SOURCE_FILES))
EXECUTABLE = $(EXECUTABLES_DIRECTORY)/paroliere_cl

# Create bin and build directories
$(EXECUTABLES_DIRECTORY):
	mkdir -p $(EXECUTABLES_DIRECTORY)

$(OBJECTS_DIRECTORY):
	mkdir -p $(OBJECTS_DIRECTORY)

# Link object files to create the executable
$(EXECUTABLE): $(OBJECT_FILES)
	$(COMPILER) $(OBJECT_FILES) -o $(EXECUTABLE)

# Compile source files to object files
$(OBJECTS_DIRECTORY)/%.o: $(SOURCE_DIRECTORY)/%.c
	$(COMPILER) $(COMP_FLAGS) -MMD -MP -c $< -o $@

# Include all .d files
-include $(OBJECTS_DIRECTORY)/*.d

# Phony Targets
.PHONY: all all_dev clean directories clear

all: directories $(EXECUTABLE)

all_dev: clear all
	@echo "Build successful!"
	@$(EXECUTABLE) localhost 8001

# Force rebuild for all_dev
.PHONY: force_rebuild
force_rebuild:
	@touch $(SOURCE_FILES) $(HEADER_FILES)

all_dev: force_rebuild

clear:
	clear

clean:
	rm -rf $(OBJECTS_DIRECTORY) $(EXECUTABLES_DIRECTORY)

directories: $(EXECUTABLES_DIRECTORY) $(OBJECTS_DIRECTORY)