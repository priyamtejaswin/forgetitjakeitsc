# # Makefile for MyAdder project

# # Compiler and compiler flags
# CXX := g++
# CXXFLAGS := -std=c++11 -Wall

# # Source files
# SRC_FILES := main.cpp my_adder.cpp

# # Object files
# OBJ_FILES := $(SRC_FILES:.cpp=.o)

# # Executable
# EXECUTABLE := my_adder.bin

# # Build rule for the executable
# $(EXECUTABLE): $(OBJ_FILES)
# 	$(CXX) $(CXXFLAGS) $^ -o $@

# # Rule for compiling source files to object files
# %.o: %.cpp
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# # Clean rule to remove generated files
# clean:
# 	rm -f $(EXECUTABLE) $(OBJ_FILES)

demo.bin : my_adder.o demo.o f_my_adder.o
	g++ demo.o my_adder.o f_my_adder.o -o demo.bin

my_adder.o : my_adder.hpp my_adder.cpp
	g++ -Wall -c my_adder.cpp -o my_adder.o

demo.o : f_my_adder.hpp Demo.c
	gcc -Wall -c demo.c -o demo.o

f_my_adder.o : f_my_adder.hpp f_my_adder.cpp my_adder.hpp
	g++ -Wall -c f_my_adder.cpp -o f_my_adder.o