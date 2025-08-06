# Define the compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -fopenmp
CLIBS = -lSDL2 -lSDL2_ttf -lm

# Define the executable name
TARGET = sdl_fun

# Find all .c files in the current directory
SRCS = $(wildcard *.c)

# Create a list of .o files from the .c files
OBJS = $(SRCS:.c=.o)

# Default target
all: run 

# Link the object files to create the final executable
build: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(CLIBS) 

# Compile the .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

run: clean build
	./sdl_fun

# Phony targets
.PHONY: all clean

