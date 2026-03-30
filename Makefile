CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = lexer

SRCS = src/main.cpp src/Lexer.cpp src/Token.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe