#include "SymbolTable.hpp"

SymbolTable::SymbolTable() : nextIndex(0) {
    enterScope();
    
    SymbolInfo intType("integer", SymbolKind::TYPE);
    intType.type = ExprType::INTEGER;
    intType.tabIndex = nextIndex++;
    intType.level = 0;
    symbolsList.push_back(intType);
    scopes.back().emplace("integer", intType);
    
    SymbolInfo realType("real", SymbolKind::TYPE);
    realType.type = ExprType::REAL;
    realType.tabIndex = nextIndex++;
    realType.level = 0;
    symbolsList.push_back(realType);
    scopes.back().emplace("real", realType);
    
    SymbolInfo charType("char", SymbolKind::TYPE);
    charType.type = ExprType::CHAR;
    charType.tabIndex = nextIndex++;
    charType.level = 0;
    symbolsList.push_back(charType);
    scopes.back().emplace("char", charType);
    
    SymbolInfo boolType("boolean", SymbolKind::TYPE);
    boolType.type = ExprType::BOOLEAN;
    boolType.tabIndex = nextIndex++;
    boolType.level = 0;
    symbolsList.push_back(boolType);
    scopes.back().emplace("boolean", boolType);
    
    SymbolInfo strType("string", SymbolKind::TYPE);
    strType.type = ExprType::STRING;
    strType.tabIndex = nextIndex++;
    strType.level = 0;
    symbolsList.push_back(strType);
    scopes.back().emplace("string", strType);
    
    SymbolInfo trueConst("true", SymbolKind::CONSTANT);
    trueConst.type = ExprType::BOOLEAN;
    trueConst.tabIndex = nextIndex++;
    trueConst.level = 0;
    symbolsList.push_back(trueConst);
    scopes.back().emplace("true", trueConst);
    
    SymbolInfo falseConst("false", SymbolKind::CONSTANT);
    falseConst.type = ExprType::BOOLEAN;
    falseConst.tabIndex = nextIndex++;
    falseConst.level = 0;
    symbolsList.push_back(falseConst);
    scopes.back().emplace("false", falseConst);
    
    SymbolInfo writeln("writeln", SymbolKind::PROCEDURE);
    writeln.type = ExprType::VOID;
    writeln.tabIndex = nextIndex++;
    writeln.level = 0;
    symbolsList.push_back(writeln);
    scopes.back().emplace("writeln", writeln);
    
    SymbolInfo write("write", SymbolKind::PROCEDURE);
    write.type = ExprType::VOID;
    write.tabIndex = nextIndex++;
    write.level = 0;
    symbolsList.push_back(write);
    scopes.back().emplace("write", write);
    
    SymbolInfo readln("readln", SymbolKind::PROCEDURE);
    readln.type = ExprType::VOID;
    readln.tabIndex = nextIndex++;
    readln.level = 0;
    symbolsList.push_back(readln);
    scopes.back().emplace("readln", readln);
    
    SymbolInfo read("read", SymbolKind::PROCEDURE);
    read.type = ExprType::VOID;
    read.tabIndex = nextIndex++;
    read.level = 0;
    symbolsList.push_back(read);
    scopes.back().emplace("read", read);
    
    SymbolInfo absFunc("abs", SymbolKind::FUNCTION);
    absFunc.type = ExprType::INTEGER; // atau bisa REAL tergantung input
    absFunc.tabIndex = nextIndex++;
    absFunc.level = 0;
    symbolsList.push_back(absFunc);
    scopes.back().emplace("abs", absFunc);
    
    SymbolInfo sqrtFunc("sqrt", SymbolKind::FUNCTION);
    sqrtFunc.type = ExprType::REAL;
    sqrtFunc.tabIndex = nextIndex++;
    sqrtFunc.level = 0;
    symbolsList.push_back(sqrtFunc);
    scopes.back().emplace("sqrt", sqrtFunc);
    
    SymbolInfo lenFunc("length", SymbolKind::FUNCTION);
    lenFunc.type = ExprType::INTEGER;
    lenFunc.tabIndex = nextIndex++;
    lenFunc.level = 0;
    symbolsList.push_back(lenFunc);
    scopes.back().emplace("length", lenFunc);

    std::vector<std::string> reserved = {
        "and","array","begin","case","const","div","downto","do","else","end",
        "for","function","if","mod","not","of","or","procedure","program","record",
        "repeat","integer","real","boolean","char","string","then","to","type",
        "until","var","while"
    };

    for (const auto& kw : reserved) {
        if (scopes.back().find(kw) != scopes.back().end()) continue;
        SymbolInfo info(kw, SymbolKind::RESERVED);
        info.tabIndex = nextIndex++;
        info.level = 0;
        symbolsList.push_back(info);
        scopes.back().emplace(kw, info);
    }
}

void SymbolTable::enterScope() {
    scopes.emplace_back();
}

void SymbolTable::exitScope() {
    if (scopes.size() > 1) {
        scopes.pop_back();
    }
}

bool SymbolTable::declare(const std::string& name, SymbolInfo info, int width) {
    if (scopes.empty()) enterScope();

    auto& currentScope = scopes.back();

    if (currentScope.find(name) != currentScope.end()) {
        return false;
    }

    info.name = name;
    info.tabIndex = nextIndex;
    info.level = static_cast<int>(scopes.size()) - 1;

    symbolsList.push_back(info);
    
    currentScope.emplace(name, info);

    nextIndex += width;

    return true;
}

SymbolInfo* SymbolTable::lookup(const std::string& name){
    for (int i = static_cast<int>(scopes.size()) - 1; i >= 0; i--) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) {
            return &it->second;
        }
    }
    return nullptr;
}

SymbolInfo* SymbolTable::lookUpCurrent(const std::string& name) {
    if (scopes.empty()) return nullptr;

    auto& currentScope = scopes.back();
    auto it = currentScope.find(name);

    if (it != currentScope.end()) {
        return &it->second;
    }
    return nullptr;
}

int SymbolTable::currentLevel() const {
    if (scopes.empty()) return -1;
    return static_cast<int>(scopes.size()) - 1;
}

int SymbolTable::nextTabIndex() const {
    return nextIndex;
}

const std::vector<SymbolInfo>& SymbolTable::allSymbols() const {
    return symbolsList;
}

int SymbolTable::createBlock(const std::string& name, int parentBlock, int level, int startTabIndex) {
    BlockInfo info;
    info.blockIndex = nextBlockIndex++;
    info.name = name;
    info.parentBlock = parentBlock;
    info.level = level;
    info.startTabIndex = startTabIndex;
    info.endTabIndex = -1;

    blocksList.push_back(info);
    return info.blockIndex;
}

void SymbolTable::closeBlock(int blockIndex, int endTabIndex) {
    if (blockIndex < 0 || blockIndex >= static_cast<int>(blocksList.size())) return;
    blocksList[blockIndex].endTabIndex = endTabIndex;
}

const BlockInfo* SymbolTable::getBlock(int blockIndex) const {
    if (blockIndex < 0 || blockIndex >= static_cast<int>(blocksList.size())) return nullptr;
    return &blocksList[blockIndex];
}

const std::vector<BlockInfo>& SymbolTable::getAllBlocks() const {
    return blocksList;
}

int SymbolTable::createArray(
    ExprType indexType,
    ExprType elementType,
    ASTNode* indexTypeNode,
    ASTNode* elementTypeNode,
    bool hasStaticBounds,
    int lowerBound,
    int upperBound,
    int elemWidth
) {
    ArrayInfo info;
    info.arrayIndex = nextArrayIndex++;
    info.indexType = indexType;
    info.elementType = elementType;
    info.indexTypeNode = indexTypeNode;
    info.elementTypeNode = elementTypeNode;
    info.hasStaticBounds = hasStaticBounds;
    info.lowerBound = lowerBound;
    info.upperBound = upperBound;
    info.elemWidth = elemWidth;

    arraysList.push_back(info);
    return info.arrayIndex;
}

const ArrayInfo* SymbolTable::getArray(int arrayIndex) const {
    if (arrayIndex < 0 || arrayIndex >= static_cast<int>(arraysList.size())) return nullptr;
    return &arraysList[arrayIndex];
}

const std::vector<ArrayInfo>& SymbolTable::getAllArrays() const {
    return arraysList;
}