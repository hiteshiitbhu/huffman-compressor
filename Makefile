CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -Isrc

SRC = src/main.cpp src/huffman.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = huffman

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)

.PHONY: all clean
