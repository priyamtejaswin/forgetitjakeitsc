CXX := g++
CXXFLAGS := -std=c++11 -Wall
TARGET := runme.bin
# Linking flags
LDFLAGS := -lpthread

# Tests
TEST_DIR := tests
TEST_FILES := $(wildcard $(TEST_DIR)/*.cc)
TEST_OBJ_FILES := $(patsubst $(TEST_DIR)/%.cc, $(TEST_DIR)/%.o, $(TEST_FILES))

# Will run by default.
$(TARGET): st5.o main.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^ $(LDFLAGS)

st5.o: st5.cc st5.h nmt_nfkc_rules.h
	$(CXX) $(CXXFLAGS) -c -o st5.o st5.cc

main.o: main.cc st5.h
	$(CXX) $(CXXFLAGS) -c -o main.o main.cc

$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I ./ -c $< -o $@

test: $(TEST_OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_DIR)/test_runner $(LDFLAGS)
	./$(TEST_DIR)/test_runner

# Won't run unless called, because it's not the first.
clean:
	rm -rf *.o $(TARGET) $(TEST_DIR)/*.o $(TEST_DIR)/test_runner
