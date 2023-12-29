CXX := g++
CXXFLAGS := -std=c++11 -Wall -g
TARGET := runme.bin

# Will run by default.
$(TARGET): st5.o main.o
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^

st5.o: st5.cc st5.h nmt_nfkc_rules.h
	$(CXX) $(CXXFLAGS) -c -o st5.o st5.cc

main.o: main.cc st5.h
	$(CXX) $(CXXFLAGS) -c -o main.o main.cc

# Won't run unless called, because it's not the first.
clean:
	rm -rf *.o $(TARGET)
