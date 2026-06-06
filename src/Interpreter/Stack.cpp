#include "Stack.hpp"

#include <stdexcept>

void Stack::reset() {
    operandStack.clear();
    dataMemory.clear();
    callFrames.clear();
}

void Stack::push(int value) {
    operandStack.push_back(value);
}

int Stack::pop() {
    if (operandStack.empty()) {
        throw std::runtime_error("Stack underflow");
    }

    int value = operandStack.back();
    operandStack.pop_back();
    return value;
}

int Stack::peek(std::size_t depth) const {
    if (depth >= operandStack.size()) {
        throw std::runtime_error("Stack peek out of range");
    }

    return operandStack[operandStack.size() - 1 - depth];
}

std::size_t Stack::stackSize() const {
    return operandStack.size();
}

void Stack::ensureDataIndex(std::size_t index) {
    if (index >= dataMemory.size()) {
        dataMemory.resize(index + 1, Cell{0, false});
    }
}

void Stack::allocate(std::size_t count) {
    std::size_t currentFrameBase = callFrames.empty() ? 0 : callFrames.back().baseAddress;
    std::size_t requiredSize = currentFrameBase + count;
    if (requiredSize > dataMemory.size()) {
        dataMemory.resize(requiredSize, Cell{0, false});
    }
}

void Stack::store(std::size_t address, int value) {
    ensureDataIndex(address);
    if (dataMemory[address].readOnly) {
        throw std::runtime_error("Attempt to modify read-only memory (CONST violation)");
    }
    dataMemory[address].value = value;
}

int Stack::load(std::size_t address) const {
    if (address >= dataMemory.size()) {
        return 0;
    }
    return dataMemory[address].value;
}

void Stack::setReadOnly(std::size_t address, bool readOnly) {
    ensureDataIndex(address);
    dataMemory[address].readOnly = readOnly;
}

std::size_t Stack::dataSize() const {
    return dataMemory.size();
}

void Stack::pushFrame(std::size_t returnAddress, std::size_t lexicalLevel, std::size_t staticLink) {
    std::size_t currentBase = dataMemory.size();
    callFrames.push_back(CallFrame{returnAddress, lexicalLevel, currentBase, staticLink});
}

bool Stack::hasFrame() const {
    return !callFrames.empty();
}

std::size_t Stack::returnAddress() const {
    if (callFrames.empty()) {
        return 0;
    }

    return callFrames.back().returnAddress;
}

std::size_t Stack::lexicalLevel() const {
    if (callFrames.empty()) {
        return 0;
    }

    return callFrames.back().lexicalLevel;
}

void Stack::popFrame() {
    if (!callFrames.empty()) {
        std::size_t oldBase = callFrames.back().baseAddress;
        callFrames.pop_back();
        dataMemory.resize(oldBase);
    }
}

std::size_t Stack::resolveAbsoluteAddress(int levelDiff, int tabIndex) const {
    if (callFrames.empty()) {
        return static_cast<std::size_t>(tabIndex);
    }

    std::size_t base = callFrames.back().baseAddress;
    std::size_t staticLnk = callFrames.back().staticLink;

    for (int i = 0; i < levelDiff; ++i) {
        base = staticLnk;
        bool found = false;
        for (auto it = callFrames.rbegin(); it != callFrames.rend(); ++it) {
            if (it->baseAddress == base) {
                staticLnk = it->staticLink;
                found = true;
                break;
            }
        }
        if (!found) break; 
    }

    return base + static_cast<std::size_t>(tabIndex);
}

std::size_t Stack::computeStaticLink(int levelDiff) const {
    if (callFrames.empty()) return 0;
    
    if (levelDiff < 0) {
        return callFrames.back().baseAddress;
    } else {
        std::size_t staticLnk = callFrames.back().staticLink;
        for (int i = 0; i < levelDiff; ++i) {
            bool found = false;
            for (auto it = callFrames.rbegin(); it != callFrames.rend(); ++it) {
                if (it->baseAddress == staticLnk) {
                    staticLnk = it->staticLink;
                    found = true;
                    break;
                }
            }
            if (!found) break;
        }
        return staticLnk;
    }
}
