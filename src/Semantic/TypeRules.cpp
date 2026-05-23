#include "TypeRules.hpp"

namespace TypeRules {
    bool isNumeric(ExprType t) {
        return t == ExprType::INTEGER || t == ExprType::REAL;
    }

    bool isInteger(ExprType t) { return t == ExprType::INTEGER; }
    bool isReal(ExprType t) { return t == ExprType::REAL; }
    bool isBoolean(ExprType t) { return t == ExprType::BOOLEAN; }
    bool isChar(ExprType t) { return t == ExprType::CHAR; }
    bool isString(ExprType t) { return t == ExprType::STRING; }

    // Cast / assign rules
    bool canImplicitCast(ExprType from, ExprType to) {
        if (from == to) return true;
        if (from == ExprType::INTEGER && to == ExprType::REAL) return true;
        if (from == ExprType::REAL && to == ExprType::INTEGER) return true;
        return false;
    }

    bool isAssignable(ExprType lhs, ExprType rhs) {
        if (lhs == ExprType::UNKNOWN || rhs == ExprType::UNKNOWN) return false;
        if (lhs == rhs) return true;
        return canImplicitCast(rhs, lhs); // RHS -> LHS
    }

    // Binary / unary results 
    ExprType resultOfBinary(const std::string& op, ExprType left, ExprType right) {
        if (op == "div" || op == "mod") {
            if (isInteger(left) && isInteger(right)) return ExprType::INTEGER;
            return ExprType::UNKNOWN;
        }

        if (op == "+" || op == "-" || op == "*" || op == "/") {
            if (isNumeric(left) && isNumeric(right)) {
                if (left == ExprType::REAL || right == ExprType::REAL || op == "/") return ExprType::REAL;
                return ExprType::INTEGER;
            }
            // String concatenation
            if (op == "+" && isString(left) && isString(right)) return ExprType::STRING;
            return ExprType::UNKNOWN;
        }

        if (op == "and" || op == "or") {
            if (isBoolean(left) && isBoolean(right)) return ExprType::BOOLEAN;
            return ExprType::UNKNOWN;
        }

        if (op == "=" || op == "==" || op == "<>" || op == "<" || op == ">" || op == "<=" || op == ">=") {
            ExprType r = resultOfRelational(left, right);
            return r == ExprType::BOOLEAN ? ExprType::BOOLEAN : ExprType::UNKNOWN;
        }

        return ExprType::UNKNOWN;
    }

    ExprType resultOfUnary(const std::string& op, ExprType operand) {
        if (op == "+" || op == "-") {
            return isNumeric(operand) ? operand : ExprType::UNKNOWN;
        }
        if (op == "not") {
            return isBoolean(operand) ? ExprType::BOOLEAN : ExprType::UNKNOWN;
        }
        return ExprType::UNKNOWN;
    }

    ExprType resultOfRelational(ExprType left, ExprType right) {
        if (isNumeric(left) && isNumeric(right)) return ExprType::BOOLEAN;
        if (left == right && (isChar(left) || isString(left) || left == ExprType::ENUM)) return ExprType::BOOLEAN;
        // Bandingin integer dan real pake promotion
        if ((isInteger(left) && isReal(right)) || (isReal(left) && isInteger(right))) return ExprType::BOOLEAN;
        return ExprType::UNKNOWN;
    }

    ExprType resultOfLogical(ExprType left, ExprType right) {
        if (isBoolean(left) && isBoolean(right)) return ExprType::BOOLEAN;
        return ExprType::UNKNOWN;
    }

    // Cek index
    bool isValidArrayIndexType(ExprType idxType) {
        return isInteger(idxType) || isChar(idxType) || idxType == ExprType::ENUM || idxType == ExprType::SUBRANGE;
    }
}