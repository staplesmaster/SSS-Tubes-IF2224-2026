#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <string>
#include <vector>
#include <ostream>

enum class PCodeOp {
    LIT,
    LOD,
    STO,
    CAL,
    INT,
    JMP,
    JPC,
    OPR,
    RET,
    LABEL,
    NOP
};

enum class OprCode {
    NEG = 1,
    ADD = 2,
    SUB = 3,
    MUL = 4,
    DIV = 5,
    MOD = 6,
    EQL = 7,
    NEQ = 8,
    LSS = 9,
    GEQ = 10,
    GTR = 11,
    LEQ = 12,
    WRT = 13,
    WRTLN = 14
};

struct Instruction {
    PCodeOp op = PCodeOp::NOP;
    int level = 0;    
    int value = 0;   
    std::string text;  
    bool hasText = false;

    Instruction() = default;
    Instruction(PCodeOp op, int level, int value);
    Instruction(PCodeOp op, const std::string& text);
};

std::string toString(PCodeOp op);
std::string toString(OprCode op);
std::string toString(const Instruction& instr);

std::ostream& operator<<(std::ostream& os, const Instruction& instr);

#endif
