CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
SRCS = $(wildcard src/*.cpp src/*/*.cpp)
INCLUDES = $(addprefix -I, $(sort $(dir $(SRCS))))

TARGET = parser

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe