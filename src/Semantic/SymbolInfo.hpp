#pragma once

#include <string>
#include <vector>
#include "ASTNode.hpp"

enum class SymbolKind {
    VARIABLE,
    CONSTANT,
    FUNCTION,
    PROCEDURE,
    TYPE, 
    PARAMETER,
    PROGRAM
};

struct ParamInfo {
    std::string name;
    ExprType type = ExprType::UNKNOWN;
    int enumDomainId = -1;

    bool isVarParam = false;
    int declLine = 0;
};

struct SymbolInfo {
    std::string name;
    SymbolKind kind;
    ExprType type = ExprType::UNKNOWN;

    int tabIndex = -1;
    int level = -1;
    int declLine = 0;

    std::vector<ParamInfo> parameters; 
    ASTNode* typeDef = nullptr;
    int enumDomainId = -1;

    bool isParameter = false;
    bool isUsed = false;
    int blockIndex = -1;
    int arrayIndex = -1;

    SymbolInfo() = default;
    SymbolInfo(const std::string& name, SymbolKind kind) : name(name), kind(kind) {}
};