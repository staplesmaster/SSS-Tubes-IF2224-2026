#include "Instruction.hpp"
#include <sstream>

Instruction::Instruction(PCodeOp op, int level, int value)
    : op(op), level(level), value(value), hasText(false) {}

Instruction::Instruction(PCodeOp op, const std::string& text)
    : op(op), level(0), value(0), text(text), hasText(true) {}

std::string toString(PCodeOp op) {
    switch (op) {
        case PCodeOp::LIT: return "LIT";
        case PCodeOp::LOD: return "LOD";
        case PCodeOp::STO: return "STO";
        case PCodeOp::CAL: return "CAL";
        case PCodeOp::INT: return "INT";
        case PCodeOp::JMP: return "JMP";
        case PCodeOp::JPC: return "JPC";
        case PCodeOp::OPR: return "OPR";
        case PCodeOp::RET: return "RET";
    }
    return "";
}

std::string toString(OprCode op) {
    switch (op) {
        case OprCode::NEG: return "NEG";
        case OprCode::ADD: return "ADD";
        case OprCode::SUB: return "SUB";
        case OprCode::MUL: return "MUL";
        case OprCode::DIV: return "DIV";
        case OprCode::MOD: return "MOD";
        case OprCode::EQL: return "EQL";
        case OprCode::NEQ: return "NEQ";
        case OprCode::LSS: return "LSS";
        case OprCode::GEQ: return "GEQ";
        case OprCode::GTR: return "GTR";
        case OprCode::LEQ: return "LEQ";
        case OprCode::WRT: return "WRT";
        case OprCode::WRTLN: return "WRTLN";
    }
    return "WRT";
}

std::string toString(const Instruction& instr) {
    std::ostringstream out;
    switch (instr.op) {
        case PCodeOp::RET:
            out << "RET";
            break;
        default:
            out << toString(instr.op) << " " << instr.level << " " << instr.value;
            if (instr.hasText) {
                out << ";'" << instr.text << "'";
            }
    }
    return out.str();
}

std::ostream& operator<<(std::ostream& os, const Instruction& instr) {
    os << toString(instr);
    return os;
}
