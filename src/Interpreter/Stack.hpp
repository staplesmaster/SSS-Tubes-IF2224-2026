#pragma once

#include <cstddef>
#include <vector>

struct CallFrame {
    std::size_t returnAddress = 0;
    std::size_t lexicalLevel = 0;
    std::size_t baseAddress = 0;
};

class Stack {
public:
    void reset();

    void push(int value);
    int pop();
    int peek(std::size_t depth = 0) const;
    std::size_t stackSize() const;

    void allocate(std::size_t count);
    void store(std::size_t address, int value);
    int load(std::size_t address) const;
    std::size_t dataSize() const;

    std::size_t resolveAbsoluteAddress(int levelDiff, int tabIndex) const;

    void pushFrame(std::size_t returnAddress, std::size_t lexicalLevel = 0);
    bool hasFrame() const;
    std::size_t returnAddress() const;
    std::size_t lexicalLevel() const;
    void popFrame();

private:
    std::vector<int> operandStack;
    std::vector<int> dataMemory;
    std::vector<CallFrame> callFrames;

    void ensureDataIndex(std::size_t index);
};
