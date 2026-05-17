#pragma once

#include "AST/ASTNode.hpp"
#include <string>

namespace TypeRules {
    bool isNumeric(ExprType t);
    bool isInteger(ExprType t);
    bool isReal(ExprType t);
    bool isBoolean(ExprType t);
    bool isChar(ExprType t);
    bool isString(ExprType t);

    // Cast / assign rules
    bool canImplicitCast(ExprType from, ExprType to); // misal INTEGER -> REAL
    bool isAssignable(ExprType lhs, ExprType rhs);

    // Binary / unary results 
    // Kalo ga allowed, nnt return nya ExprType::UNKNOWN 
    ExprType resultOfBinary(const std::string& op, ExprType left, ExprType right);
    ExprType resultOfUnary(const std::string& op, ExprType operand);
    ExprType resultOfRelational(ExprType left, ExprType right);
    ExprType resultOfLogical(ExprType left, ExprType right);

    // Cek index
    bool isValidArrayIndexType(ExprType idxType);
}