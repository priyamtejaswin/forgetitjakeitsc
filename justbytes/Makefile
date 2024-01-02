CXX := g++
CXXFLAGS := -std=c++11 -Wall
TARGET := runme.bin
TESTER := testme.bin
LDFLAGS := -lpthread
INCLUDES := -I .

BUILD_DIR := build

TEST_FILES := $(shell find .  -name 'test_*.cc')
TEST_OBJ_FILES := $(patsubst %.cc, $(BUILD_DIR)/%.o, $(TEST_FILES))

# Src
SRC_FILES := st5.cc main.cc
SRC_OBJ_FILES := $(patsubst %.cc, $(BUILD_DIR)/%.o, $(SRC_FILES))

# $(info SRC_FILES is $(SRC_FILES))
# $(info SRC_OBJ_FILES is $(SRC_OBJ_FILES))

target: $(BUILD_DIR) $(TARGET)

$(TARGET): $(SRC_OBJ_FILES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(SRC_OBJ_FILES): $(wildcard $(INCLUDES)/*.h)

$(BUILD_DIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Test
test: $(BUILD_DIR) $(TESTER)

$(TESTER): $(TEST_OBJ_FILES) $(BUILD_DIR)/st5.o
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@
	./$(TESTER)

$(TEST_OBJ_FILES): $(wildcard $(INCLUDES)/*.h)

$(BUILD_DIR)/%.o: test_%.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf *.bin