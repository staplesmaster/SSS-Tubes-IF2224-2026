#include "VM.hpp"

#include <stdexcept>

VM::VM(const std::vector<Instruction>& program, std::istream& input, std::ostream& output)
    : program(&program), input(&input), output(&output), pc(0), stopped(false) {}

void VM::reset() {
    stack.reset();
    pc = 0;
    stopped = false;
}

bool VM::halted() const {
    return stopped;
}

std::size_t VM::programCounter() const {
    return pc;
}

const Stack& VM::getStack() const {
    return stack;
}

Stack& VM::getStack() {
    return stack;
}

void VM::run() {
    if (!program) {
        throw std::runtime_error("VM has no program to execute");
    }

    stopped = false;
    while (!stopped && pc < program->size()) {
        const Instruction& instruction = (*program)[pc];
        execute(instruction);
    }
}

void VM::execute(const Instruction& instruction) {
    switch (instruction.op) {
        case PCodeOp::LIT:
            stack.push(instruction.value);
            ++pc;
            break;

        case PCodeOp::LOD:
            {
                std::size_t addr = stack.resolveAbsoluteAddress(instruction.level, instruction.value);
                stack.push(stack.load(addr));
            }
            ++pc;
            break;

        case PCodeOp::STO: {
            int value = stack.pop();
            std::size_t addr = stack.resolveAbsoluteAddress(instruction.level, instruction.value);
            stack.store(addr, value);
            ++pc;
            break;
        }

        case PCodeOp::LODA: {
            std::size_t addr = stack.resolveAbsoluteAddress(instruction.level, instruction.value);
            stack.push(static_cast<int>(addr));
            ++pc;
            break;
        }

        case PCodeOp::LODI: {
            int rawAddress = stack.pop();
            if (rawAddress < 0) {
                throw std::runtime_error("Invalid memory address");
            }
            std::size_t addr = static_cast<std::size_t>(rawAddress);
            stack.push(stack.load(addr));
            ++pc;
            break;
        }

        case PCodeOp::STOI: {
            int rawAddress = stack.pop();
            if (rawAddress < 0) {
                throw std::runtime_error("Invalid memory address");
            }
            std::size_t addr = static_cast<std::size_t>(rawAddress);
            int value = stack.pop();
            stack.store(addr, value);
            ++pc;
            break;
        }

        case PCodeOp::CHK: {
            int value = stack.pop();
            int lower = instruction.level;
            int upper = instruction.value;
            if (value < lower || value > upper) {
                throw std::runtime_error("Array index out of bounds");
            }
            stack.push(value);
            ++pc;
            break;
        }

        case PCodeOp::INT:
            stack.allocate(static_cast<std::size_t>(instruction.value));
            ++pc;
            break;

        case PCodeOp::CAL:
            {
                int diff = instruction.level;
                std::size_t callerLevel = stack.lexicalLevel();
                std::size_t calleeLevel = static_cast<std::size_t>(static_cast<int>(callerLevel) - diff);
                std::size_t staticLink = stack.computeStaticLink(diff);
                stack.pushFrame(pc + 1, calleeLevel, staticLink);
                pc = static_cast<std::size_t>(instruction.value);
            }
            break;

        case PCodeOp::JMP:
            pc = static_cast<std::size_t>(instruction.value);
            break;

        case PCodeOp::JPC: {
            int condition = stack.pop();
            if (condition == 0) {
                pc = static_cast<std::size_t>(instruction.value);
            } else {
                ++pc;
            }
            break;
        }

        case PCodeOp::OPR: {
            if (instruction.hasText) {
                (*output) << instruction.text;
                ++pc;
                break;
            }

            const int op = instruction.value;
            switch (op) {
                case static_cast<int>(OprCode::NEG): {
                    int value = stack.pop();
                    stack.push(-value);
                    break;
                }
                case static_cast<int>(OprCode::ADD): {
                    int rhs = stack.pop();
                    int lhs = stack.pop();
                    stack.push(lhs + rhs);
                    break;
                }
                case static_cast<int>(OprCode::SUB): {
                    int rhs = stack.pop();
                    int lhs = stack.pop();
                    stack.push(lhs - rhs);
                    break;
                }
                case static_cast<int>(OprCode::MUL): {
                    int rhs = stack.pop();
                    int lhs = stack.pop();
                    stack.push(lhs * rhs);
                    break;
                }
                case static_cast<int>(OprCode::DIV): {
                    int rhs = stack.pop();
                    int lhs = stack.pop();
                    if (rhs == 0) {
                        throw std::runtime_error("Division by zero");
                    }
                    stack.push(lhs / rhs);
                    break;
                }
                case static_cast<int>(OprCode::MOD): {
                    int rhs = stack.pop();
                    int lhs = stack.pop();
                    if (rhs == 0) {
                        throw std::runtime_error("Modulo by zero");
                    }
                    stack.push(lhs % rhs);
                    break;
                }
                case static_cast<int>(OprCode::EQL): {
                    int rhs = stack.pop();
                    int lhs = stack.pop();
                    stack.push(lhs == rhs ? 1 : 0);
                    break;
                }
                case static_cast<int>(OprCode::NEQ): {
                    int rhs = stack.pop();
                    int lhs = stack.pop();
                    stack.push(lhs != rhs ? 1 : 0);
                    break;
                }
                case static_cast<int>(OprCode::LSS): {
                    int rhs = stack.pop();
                    int lhs = stack.pop();
                    stack.push(lhs < rhs ? 1 : 0);
                    break;
                }
                case static_cast<int>(OprCode::GEQ): {
                    int rhs = stack.pop();
                    int lhs = stack.pop();
                    stack.push(lhs >= rhs ? 1 : 0);
                    break;
                }
                case static_cast<int>(OprCode::GTR): {
                    int rhs = stack.pop();
                    int lhs = stack.pop();
                    stack.push(lhs > rhs ? 1 : 0);
                    break;
                }
                case static_cast<int>(OprCode::LEQ): {
                    int rhs = stack.pop();
                    int lhs = stack.pop();
                    stack.push(lhs <= rhs ? 1 : 0);
                    break;
                }
                case static_cast<int>(OprCode::WRT): {
                    (*output) << stack.pop();
                    break;
                }
                case static_cast<int>(OprCode::WRTBOOL): {
                    int val = stack.pop();
                    (*output) << (val ? "true" : "false");
                    break;
                }
                case static_cast<int>(OprCode::WRTLN): {
                    (*output) << '\n';
                    break;
                }
                case static_cast<int>(OprCode::AND): {
                    int rhs = stack.pop();
                    int lhs = stack.pop();
                    stack.push((lhs && rhs) ? 1 : 0);
                    break;
                }
                case static_cast<int>(OprCode::OR): {
                    int rhs = stack.pop();
                    int lhs = stack.pop();
                    stack.push((lhs || rhs) ? 1 : 0);
                    break;
                }
                default:
                    throw std::runtime_error("Unsupported OPR code");
            }
            ++pc;
            break;
        }

        case PCodeOp::RET:
            if (stack.hasFrame()) {
                pc = stack.returnAddress();
                stack.popFrame();
            } else {
                stopped = true;
            }
            break;
    }
}
