# Makefile for MyAdder project

# Compiler and compiler flags
CXX := g++
CXXFLAGS := -std=c++11 -Wall

# Source files
SRC_FILES := main.cpp my_adder.cpp

# Object files
OBJ_FILES := $(SRC_FILES:.cpp=.o)

# Executable
EXECUTABLE := my_adder.bin

# Build rule for the executable
$(EXECUTABLE): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Rule for compiling source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule to remove generated files
clean:
	rm -f $(EXECUTABLE) $(OBJ_FILES)