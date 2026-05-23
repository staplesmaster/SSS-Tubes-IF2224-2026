#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "SymbolInfo.hpp"

struct BlockInfo {
    int blockIndex = -1;
    std::string name;
    int parentBlock = -1;
    int level = -1;
    int startTabIndex = -1;
    int endTabIndex = -1;
};

struct ArrayInfo {
    int arrayIndex = -1;
    ExprType elementType = ExprType::UNKNOWN;
    ExprType indexType = ExprType::UNKNOWN;

    ASTNode* indexTypeNode = nullptr; 
    ASTNode* elementTypeNode = nullptr;

    int lowerBound = 0;
    int upperBound = 0;
    bool hasStaticBounds = false;
};

class SymbolTable {
    private:
        std::vector<std::unordered_map<std::string, SymbolInfo>> scopes;
        std::vector<SymbolInfo> symbolsList;
        int nextIndex; 

        std::vector<BlockInfo> blocksList;
        std::vector<ArrayInfo> arraysList;
        int nextBlockIndex = 0;
        int nextArrayIndex = 0;
    public:
        SymbolTable();

        void enterScope();
        void exitScope();

        bool declare(const std::string& name, SymbolInfo info);
        SymbolInfo* lookup(const std::string& name);
        SymbolInfo* lookUpCurrent(const std::string& name);

        int currentLevel() const;
        int nextTabIndex() const;
        const std::vector<SymbolInfo>& allSymbols() const;


        int createBlock(const std::string& name, int parentBlock, int level, int startTabIndex);
        void closeBlock(int blockIndex, int endTabIndex);
        const BlockInfo* getBlock(int blockIndex) const;
        const std::vector<BlockInfo>& getAllBlocks() const;

        int createArray(
            ExprType indexType,
            ExprType elementType,
            ASTNode* indexTypeNode = nullptr,
            ASTNode* elementTypeNode = nullptr,
            bool hasStaticBounds = false,
            int lowerBound = 0,
            int upperBound = 0
        );
        const ArrayInfo* getArray(int arrayIndex) const;
        const std::vector<ArrayInfo>& getAllArrays() const;
};
