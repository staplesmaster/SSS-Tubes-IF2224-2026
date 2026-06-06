#pragma once

#include <cstddef>
#include <istream>
#include <ostream>
#include <vector>
#include <iostream>

#include "Instruction.hpp"
#include "Stack.hpp"

class VM {
public:
    explicit VM(const std::vector<Instruction>& program,
                std::istream& input = std::cin,
                std::ostream& output = std::cout);

    void run();
    void reset();

    bool halted() const;
    std::size_t programCounter() const;

    const Stack& getStack() const;
    Stack& getStack();

private:
    const std::vector<Instruction>* program;
    Stack stack;
    std::istream* input;
    std::ostream* output;
    std::size_t pc;
    bool stopped;

    void execute(const Instruction& instruction);
};
