CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
INCLUDES = -Isrc -Isrc/FileProc -Isrc/Lexer -Isrc/Parser
TARGET = lexer

SRCS = src/main.cpp src/FileProc/Reader.cpp src/FileProc/Writer.cpp src/Lexer/Lexer.cpp src/Lexer/Token.cpp src/Parser/ParseNode.cpp src/Parser/Parser.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe