# Define the compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror
CLIBS = -lSDL2 -lm

# Define the executable name
TARGET = sdl_fun

# Find all .c files in the current directory
SRCS = $(wildcard *.c)

# Create a list of .o files from the .c files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link the object files to create the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(CLIBS) 

# Compile the .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean

