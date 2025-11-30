# Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Target Executable Name
TARGET = myshell

# Recursively find all .cpp files in the current directory and subdirectories
# usage of 'shell' command to execute 'find'
SRCS := $(shell find . -name "*.cpp")

# Convert source file paths to object file paths (e.g., src/main.cpp -> src/main.o)
OBJS := $(SRCS:.cpp=.o)

# Default target: Build the executable
all: $(TARGET)

# Rule to link object files into the final executable
$(TARGET): $(OBJS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	@echo "Build successful!"

# Rule to compile .cpp files into .o files
# This generic rule works for any .cpp file found in any subdirectory
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Target to build and run the executable
run: all
	@echo "Running $(TARGET)..."
	./$(TARGET)

# Clean rule: remove generated object files, executable, and the named pipe
clean:
	@echo "Cleaning up..."
	rm -f $(OBJS) $(TARGET) np_fifo

# Phony targets to avoid conflicts with files named 'all', 'clean', or 'run'
.PHONY: all clean run