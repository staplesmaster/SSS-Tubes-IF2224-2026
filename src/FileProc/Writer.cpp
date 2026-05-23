#include "Writer.hpp"
#include "SemanticAnalyzer.hpp"
#include "SymbolInfo.hpp"
#include <fstream>
#include <functional>
#include <stdexcept>
#include <iomanip>

namespace {
int countNewline(const string& sourceCode, int start, int end) {
    enum class ScanState {
        NORMAL,
        BRACE_COMMENT,
        PAREN_COMMENT,
        PAREN_COMMENT_STAR
    };

    ScanState state = ScanState::NORMAL;
    int newlineCount = 0;
    bool hasComment = false;

    for (int i = start; i < end && i < (int)sourceCode.size(); ++i) {
        char c = sourceCode[i];

        switch (state) {
            case ScanState::NORMAL:
                if (c == '{') {
                    hasComment = true;
                    state = ScanState::BRACE_COMMENT;
                } else if (c == '(' && i + 1 < end && i + 1 < (int)sourceCode.size() && sourceCode[i + 1] == '*') {
                    hasComment = true;
                    state = ScanState::PAREN_COMMENT;
                    ++i;
                } else if (c == '\n') {
                    ++newlineCount;
                }
                break;

            case ScanState::BRACE_COMMENT:
                if (c == '}') {
                    state = ScanState::NORMAL;
                }
                break;

            case ScanState::PAREN_COMMENT:
                if (c == '*') {
                    state = ScanState::PAREN_COMMENT_STAR;
                }
                break;

            case ScanState::PAREN_COMMENT_STAR:
                if (c == ')') {
                    state = ScanState::NORMAL;
                } else if (c != '*') {
                    state = ScanState::PAREN_COMMENT;
                }
                break;
        }
    }

    if (hasComment) {
        return 0;
    }

    return newlineCount;
}

const SymbolInfo* lookupSymbol(const SymbolTable& symbolTable, const string& name) {
    return const_cast<SymbolTable&>(symbolTable).lookup(name);
}

int typeCodeForNode(const SymbolTable& symbolTable, ASTNode* typeNode);

int typeSizeForNode(const SymbolTable& symbolTable, ASTNode* typeNode);

int typeCodeForNode(const SymbolTable& symbolTable, ASTNode* typeNode) {
    if (!typeNode) return 0;

    if (auto* namedType = dynamic_cast<NamedTypeNode*>(typeNode)) {
        const SymbolInfo* info = lookupSymbol(symbolTable, namedType->typeName);
        if (info && info->kind == SymbolKind::TYPE) {
            return info->tabIndex;
        }
        return 0;
    }

    if (dynamic_cast<RangeNode*>(typeNode)) {
        const SymbolInfo* info = lookupSymbol(symbolTable, "integer");
        return (info && info->kind == SymbolKind::TYPE) ? info->tabIndex : 0;
    }

    if (dynamic_cast<EnumNode*>(typeNode)) {
        const SymbolInfo* info = lookupSymbol(symbolTable, "integer");
        return (info && info->kind == SymbolKind::TYPE) ? info->tabIndex : 0;
    }

    return 0;
}

int typeSizeFromExprType(ExprType type) {
    switch (type) {
        case ExprType::INTEGER: return 4;
        case ExprType::REAL: return 8;
        case ExprType::CHAR: return 1;
        case ExprType::BOOLEAN: return 1;
        case ExprType::STRING: return 1;
        default: return 0;
    }
}

int typeSizeForNode(const SymbolTable& symbolTable, ASTNode* typeNode) {
    if (!typeNode) return 0;

    if (auto* namedType = dynamic_cast<NamedTypeNode*>(typeNode)) {
        if (namedType->typeName == "integer") return typeSizeFromExprType(ExprType::INTEGER);
        if (namedType->typeName == "real") return typeSizeFromExprType(ExprType::REAL);
        if (namedType->typeName == "char") return typeSizeFromExprType(ExprType::CHAR);
        if (namedType->typeName == "boolean") return typeSizeFromExprType(ExprType::BOOLEAN);
        if (namedType->typeName == "string") return typeSizeFromExprType(ExprType::STRING);

        const SymbolInfo* info = lookupSymbol(symbolTable, namedType->typeName);
        if (info && info->kind == SymbolKind::TYPE) {
            if (info->typeDef && info->typeDef != typeNode) {
                return typeSizeForNode(symbolTable, info->typeDef);
            }
            return typeSizeFromExprType(info->type);
        }
        return 0;
    }

    if (auto* arrayType = dynamic_cast<ArrayTypeNode*>(typeNode)) {
        int elemSize = typeSizeForNode(symbolTable, arrayType->elementType);
        int low = 0;
        int high = 0;
        if (auto* rangeType = dynamic_cast<RangeNode*>(arrayType->indexType)) {
            auto getIntValue = [](ASTNode* node, int& value) {
                if (auto* numberNode = dynamic_cast<NumberNode*>(node)) {
                    if (numberNode->isReal) return false;
                    try {
                        value = std::stoi(numberNode->value);
                        return true;
                    } catch (...) {
                        return false;
                    }
                }
                return false;
            };

            if (getIntValue(rangeType->lowerBound, low) && getIntValue(rangeType->upperBound, high) && high >= low) {
                return (high - low + 1) * elemSize;
            }
        }
        return elemSize;
    }

    if (auto* recordType = dynamic_cast<RecordTypeNode*>(typeNode)) {
        int total = 0;
        for (ASTNode* fieldNode : recordType->fields) {
            auto* fieldDecl = dynamic_cast<VarDeclNode*>(fieldNode);
            if (!fieldDecl) continue;
            int fieldSize = typeSizeForNode(symbolTable, fieldDecl->typeDef);
            total += fieldSize * static_cast<int>(fieldDecl->getVarNames().size());
        }
        return total;
    }

    if (dynamic_cast<EnumNode*>(typeNode)) return typeSizeFromExprType(ExprType::INTEGER);
    if (dynamic_cast<RangeNode*>(typeNode)) return typeSizeFromExprType(ExprType::INTEGER);

    return typeSizeFromExprType(dynamic_cast<ASTNode*>(typeNode)->exprType);
}

int arrayDetailRef(const SymbolTable& symbolTable, ASTNode* elementTypeNode) {
    if (!elementTypeNode) return 0;

    if (auto* namedType = dynamic_cast<NamedTypeNode*>(elementTypeNode)) {
        const SymbolInfo* info = lookupSymbol(symbolTable, namedType->typeName);
        if (!info) return 0;
        if (info->kind == SymbolKind::TYPE && info->type == ExprType::ARRAY && info->arrayIndex >= 0) {
            return info->arrayIndex + 1;
        }
        if (info->kind == SymbolKind::TYPE && info->type == ExprType::RECORD) {
            return info->blockIndex >= 0 ? info->blockIndex + 1 : 0;
        }
        if (info->kind == SymbolKind::TYPE && info->typeDef) {
            return arrayDetailRef(symbolTable, info->typeDef);
        }
    }

    return 0;
}
}

void writeTokens(const string& filepath, const vector<Token>& tokens, const string& sourceCode) {
    ofstream outputFile(filepath);
    if (!outputFile.is_open()) {
        throw runtime_error("Gagal membuat file output: " + filepath);
    }

    int previousEnd = 0;
    bool firstToken = true;

    for (const Token& token : tokens) {
        int newlineCount = countNewline(sourceCode, previousEnd, token.start);

        int extraBlankLines = 0;
        if (firstToken) {
            extraBlankLines = newlineCount;
        } else if (newlineCount > 0) {
            extraBlankLines = newlineCount - 1;
        }

        for (int i = 0; i < extraBlankLines; ++i) {
            outputFile << "\n";
        }

        string typeStr = typeToString(token.type);
        string lexeme = sourceCode.substr(token.start, token.end - token.start);
        
        if (token.type == TokenType::IDENTIFIER || token.type == TokenType::INTCON || 
            token.type == TokenType::REALCON || token.type == TokenType::STRING || 
            token.type == TokenType::CHARCON || token.type == TokenType::UNKNOWN) {
            
            outputFile << typeStr << " (" << lexeme << ")\n";
            
        } else {
            outputFile << typeStr << "\n";
        }

        previousEnd = token.end;
        firstToken = false;
    }

    outputFile.close();
}

void writeTreeRecursive(ofstream& out, ParseNode* node, const string& prefix = "", bool isLast = true, bool isRoot = true) {
    if (node == nullptr) return;

    if (!isRoot) {
        out << prefix;
        out << (isLast ? "└── " : "├── "); 
    }

    if (!node->getIsTerminal()) {
        out << node->getName() << "\n";
    } else {
        string typeStr = typeToString(node->getToken().type); 
        
        if (node->getToken().type == TokenType::UNKNOWN) {
            out << node->getToken().value << "\n"; 
        } 
        else if (node->getToken().type == TokenType::IDENTIFIER || node->getToken().type == TokenType::INTCON || 
                 node->getToken().type == TokenType::REALCON || node->getToken().type == TokenType::STRING || 
                 node->getToken().type == TokenType::CHARCON) {
            out << typeStr << "(" << node->getToken().value << ")\n";
        } else {
            out << typeStr << "\n";
        }
    }

    for (size_t i = 0; i < node->getChildren().size(); ++i) {
        string newPrefix = prefix;
        if (!isRoot) {
            newPrefix += (isLast ? "    " : "│   "); 
        }
        
        bool childIsLast = (i == node->getChildren().size() - 1);
        writeTreeRecursive(out, node->getChildren()[i], newPrefix, childIsLast, false);
    }
}

void writeParseResult(const string& filepath, ParseNode* root, const vector<string>& errors) {
    ofstream outFile(filepath);
    if (!outFile.is_open()) {
        throw runtime_error("Gagal membuka file untuk menulis Parse Tree: " + filepath);
    }

    if (!errors.empty()) {
        outFile << "DAFTAR SYNTAX ERROR\n";
        for (const string& err : errors) {
            outFile << err << "\n";
        }
        outFile << "\n";
    } else {
        outFile << "TIDAK ADA SYNTAX ERROR (PROGRAM VALID)\n\n";
    }

    outFile << "HASIL PARSE TREE\n";
    if (root != nullptr) {
        writeTreeRecursive(outFile, root);
    }

    outFile.close();
}

string astToString(ASTNode* node) {
    if (!node) return "null";
    
    // Tipe Data Dasar
    if (auto* namedType = dynamic_cast<NamedTypeNode*>(node)) {
        return "'" + namedType->typeName + "'";
    }
    // Nilai Literal & Variabel
    else if (auto* varNode = dynamic_cast<VarNode*>(node)) {
        return "Var('" + varNode->name + "')"; 
    }
    else if (auto* numNode = dynamic_cast<NumberNode*>(node)) {
        return "Num(" + numNode->value + ")";
    }
    else if (auto* strNode = dynamic_cast<StringNode*>(node)) {
        return "String('" + strNode->value + "')";
    }
    else if (auto* charNode = dynamic_cast<CharNode*>(node)) {
        return "Char(" + charNode->value + ")";
    }
    // Operasi
    else if (auto* binOp = dynamic_cast<BinOpNode*>(node)) {
        return "BinOp(op: '" + binOp->op + "', left: " + astToString(binOp->left) + ", right: " + astToString(binOp->right) + ")";
    }
    else if (auto* unOp = dynamic_cast<UnaryOpNode*>(node)) {
        return "UnaryOp(op: '" + unOp->op + "', operand: " + astToString(unOp->operand) + ")";
    }
    // Akses Kompleks
    else if (auto* arrAcc = dynamic_cast<ArrayAccessNode*>(node)) {
        string res = "ArrayAccess(base: " + astToString(arrAcc->getArrayVar()) + ", indices: [";
        for (size_t i = 0; i < arrAcc->indices.size(); ++i) {
            res += astToString(arrAcc->indices[i]);
            if (i < arrAcc->indices.size() - 1) res += ", ";
        }
        return res + "])";
    }
    else if (auto* recAcc = dynamic_cast<RecordAccessNode*>(node)) {
        return "RecordAccess(base: " + astToString(recAcc->getRecordVar()) + ", field: '" + recAcc->fieldName + "')";
    }
    
    return "ComplexExpression";
}

// Helper mencetak struktur vertikal untuk Blok, Statement, dan Deklarasi
void writeASTRecursive(ofstream& out, ASTNode* node, const string& prefix = "", bool isLast = true, bool isRoot = true) {
    if (node == nullptr) return;

    if (!isRoot) {
        out << prefix;
        out << (isLast ? "└── " : "├── ");
    }

    string newPrefix = prefix + (isRoot ? "" : (isLast ? "    " : "│   "));

    // STRUKTUR PROGRAM
    if (auto* progNode = dynamic_cast<ProgramNode*>(node)) {
        out << "ProgramNode(name: '" << progNode->getProgramName() << "')\n";
        
        // Cabang Deklarasi
        if (!progNode->declarations.empty()) {
            out << newPrefix << "├── Declarations\n";
            string declPrefix = newPrefix + "│   ";
            for (size_t i = 0; i < progNode->declarations.size(); ++i) {
                bool childLast = (i == progNode->declarations.size() - 1);
                writeASTRecursive(out, progNode->declarations[i], declPrefix, childLast, false);
            }
        }
        
        // Cabang Block
        out << newPrefix << "└── Block\n";
        string blockPrefix = newPrefix + "    ";
        if (auto* mainCompound = dynamic_cast<CompoundNode*>(progNode->mainBlock)) {
            for (size_t i = 0; i < mainCompound->statements.size(); ++i) {
                bool childLast = (i == mainCompound->statements.size() - 1);
                writeASTRecursive(out, mainCompound->statements[i], blockPrefix, childLast, false);
            }
        } 
        else {
            writeASTRecursive(out, progNode->mainBlock, blockPrefix, true, false);
        }
    }
    else if (auto* compNode = dynamic_cast<CompoundNode*>(node)) {
        out << "CompoundStatement\n";
        for (size_t i = 0; i < compNode->statements.size(); ++i) {
            bool childLast = (i == compNode->statements.size() - 1);
            writeASTRecursive(out, compNode->statements[i], newPrefix, childLast, false);
        }
    }

    // DEKLARASI
    else if (auto* varDecl = dynamic_cast<VarDeclNode*>(node)) {
        for (size_t i = 0; i < varDecl->getVarNames().size(); ++i) {
            out << "VarDecl(name: '" << varDecl->getVarNames()[i] << "', type: " << astToString(varDecl->getTypeDef()) << ")";
            if (i < varDecl->getVarNames().size() - 1) {
                out << "\n" << prefix << (isLast ? "    " : "│   ");
            }
        }
        out << "\n";
    }
    else if (auto* constDecl = dynamic_cast<ConstDeclNode*>(node)) {
        out << "ConstDecl(name: '" << constDecl->getConstName() << "', value: " << astToString(constDecl->getValue()) << ")\n";
    }
    else if (auto* typeDecl = dynamic_cast<TypeDeclNode*>(node)) {
        out << "TypeDecl(name: '" << typeDecl->getTypeName() << "')\n";
    }
    else if (auto* subDecl = dynamic_cast<SubprogramDeclNode*>(node)) {
        out << (subDecl->isFunction ? "FunctionDecl" : "ProcedureDecl") 
            << "(name: '" << subDecl->getSubprogramName() << "')\n";
        // Cetak blok subprogram jika ada
        if (subDecl->getBody()) {
            writeASTRecursive(out, subDecl->getBody(), newPrefix, true, false);
        }
    }

    // STATEMENT
    else if (auto* assignNode = dynamic_cast<AssignNode*>(node)) {
        out << "Assign(target: " << astToString(assignNode->target) 
            << ", value: " << astToString(assignNode->value) << ")\n";
    }
    else if (auto* procCall = dynamic_cast<ProcCallNode*>(node)) {
        out << "ProcedureCall(name: '" << procCall->procName << "', args: [";
        for (size_t i = 0; i < procCall->getArguments().size(); ++i) {
            out << astToString(procCall->getArguments()[i]);
            if (i < procCall->getArguments().size() - 1) out << ", ";
        }
        out << "])\n";
    }
    else if (auto* ifNode = dynamic_cast<IfNode*>(node)) {
        out << "IfStatement(condition: " << astToString(ifNode->condition) << ")\n";
        
        bool hasElse = (ifNode->elseBlock != nullptr);
        out << newPrefix << (hasElse ? "├── " : "└── ") << "ThenBlock\n";
        writeASTRecursive(out, ifNode->thenBlock, newPrefix + (hasElse ? "│   " : "    "), true, false);
        
        if (hasElse) {
            out << newPrefix << "└── ElseBlock\n";
            writeASTRecursive(out, ifNode->elseBlock, newPrefix + "    ", true, false);
        }
    }
    else if (auto* whileNode = dynamic_cast<WhileNode*>(node)) {
        out << "WhileLoop(condition: " << astToString(whileNode->condition) << ")\n";
        out << newPrefix << "└── DoBlock\n";
        writeASTRecursive(out, whileNode->loopBlock, newPrefix + "    ", true, false);
    }
    else if (auto* forNode = dynamic_cast<ForNode*>(node)) {
        out << "ForLoop(iterator: '" << forNode->iteratorName 
            << "', start: " << astToString(forNode->startValue) 
            << ", end: " << astToString(forNode->endValue) 
            << ", direction: " << (forNode->isDownto ? "downto" : "to") << ")\n";
        out << newPrefix << "└── DoBlock\n";
        writeASTRecursive(out, forNode->loopBlock, newPrefix + "    ", true, false);
    }
    
    else {
        out << "ASTNode(" << astToString(node) << ")\n";
    }
}

// Fungsi Utama
void writeASTResult(const string& filepath, ASTNode* root) {
    ofstream outFile(filepath);
    if (!outFile.is_open()) {
        throw runtime_error("Gagal membuka file untuk menulis AST: " + filepath);
    }

    outFile << "HASIL ABSTRACT SYNTAX TREE (AST)\n";
    if (root != nullptr) {
        writeASTRecursive(outFile, root);
    } else {
        outFile << "AST Kosong (Terdapat Syntax Error sehingga AST tidak dibentuk).\n";
    }

    outFile.close();
}
string exprTypeToString(ExprType type) {
    switch (type) {
        case ExprType::INTEGER: return "integer";
        case ExprType::REAL: return "real";
        case ExprType::BOOLEAN: return "boolean";
        case ExprType::CHAR: return "char";
        case ExprType::STRING: return "string";
        case ExprType::VOID: return "void";
        case ExprType::ARRAY: return "array";
        case ExprType::RECORD: return "record";
        case ExprType::ENUM: return "enum";
        case ExprType::SUBRANGE: return "subrange";
        default: return "unknown";
    }
}

string symbolKindToString(SymbolKind kind) {
    switch (kind) {
        case SymbolKind::VARIABLE: return "variable";
        case SymbolKind::CONSTANT: return "constant";
        case SymbolKind::FUNCTION: return "function";
        case SymbolKind::PROCEDURE: return "procedure";
        case SymbolKind::TYPE: return "type";
        case SymbolKind::PARAMETER: return "parameter";
        case SymbolKind::PROGRAM: return "program";
        default: return "unknown";
    }
}

bool isPredefinedSymbol(const SymbolInfo& sym) {
    return sym.tabIndex >= 0 && sym.tabIndex < 33;
}

bool shouldPrintTabSymbol(const SymbolInfo& sym) {
    if (!isPredefinedSymbol(sym)) return true;
    return (sym.kind == SymbolKind::PROCEDURE || sym.kind == SymbolKind::FUNCTION) && sym.isUsed;
}

string joinMetadata(const vector<string>& items) {
    string result;
    for (const string& item : items) {
        if (!result.empty()) result += ", ";
        result += item;
    }
    return result;
}

string varDeclLine(const SymbolTable& symbolTable, const VarDeclNode* node, const string& name) {
    SymbolInfo* info = const_cast<SymbolTable&>(symbolTable).lookup(name);
    string line = "VarDecl('" + name + "')";
    if (node && node->typeDef) {
        if (auto* nt = dynamic_cast<NamedTypeNode*>(node->typeDef))
            line += ", type:" + nt->typeName;
        else
            line += ", type:" + astToString(node->typeDef);
    }
    if (info) {
        line += compactMetaSym(info);
    } else if (node) {
        line += compactMetaNode(node);
    }
    return line;
}

string tabObjectLabel(const SymbolInfo& sym) {
    if (isPredefinedSymbol(sym)) {
        return symbolKindToString(sym.kind) + " ...";
    }
    return symbolKindToString(sym.kind);
}

string compactMeta(int blockIndex, int tabIndex, ExprType exprType, int lev,
                   bool predefined) {
    vector<string> items;
    if (blockIndex >= 0)items.push_back("block_index:" + to_string(blockIndex));
    if (tabIndex >= 0)items.push_back("tab_index:" + to_string(tabIndex));
    if (exprType != ExprType::UNKNOWN) items.push_back("type:" + exprTypeToString(exprType));
    if (lev >= 0)items.push_back("lev:" + to_string(lev));
    if (predefined)items.push_back("predefined");
    if (items.empty()) return "";
    string r;
    for (auto& s : items) { if (!r.empty()) r += ", "; r += s; }
    return " → " + r;
}

string compactMetaNode(const ASTNode* node, int blockIndex, bool predefined) {
    if (!node) return blockIndex >= 0 ? (" → block_index:" + to_string(blockIndex)) : "";
    return compactMeta(blockIndex, node->tabIndex, node->exprType, node->lev, predefined);
}

string compactMetaSym(const SymbolInfo* info, int blockIndex) {
    if (!info) return blockIndex >= 0 ? (" → block_index:" + to_string(blockIndex)) : "";
    bool pred = info->declLine == 0 && info->kind != SymbolKind::PROGRAM;
    return compactMeta(blockIndex, info->tabIndex, info->type, info->level, pred);
}

string nodeLabel(ASTNode* node, const SymbolTable& symbolTable, int blockIndex = -1) {
    if (!node) return "null";

    // Program
    if (auto* progNode = dynamic_cast<ProgramNode*>(node)) {
        SymbolInfo* info = const_cast<SymbolTable&>(symbolTable).lookup(progNode->getProgramName());
        int pb = info ? info->blockIndex : blockIndex;
        return "ProgramNode('" + progNode->getProgramName() + "')" + compactMetaNode(node, pb);
    }

    // Block & Compound
    if (dynamic_cast<CompoundNode*>(node))
        return "Block" + compactMetaNode(node, blockIndex);

    // Deklarasi
    if (auto* varDecl = dynamic_cast<VarDeclNode*>(node)) {
        if (!varDecl->varNames.empty()) {
            const string& nm = varDecl->varNames.front();
            SymbolInfo* info = const_cast<SymbolTable&>(symbolTable).lookup(nm);
            string typePart;
            if (varDecl->typeDef) {
                if (auto* nt = dynamic_cast<NamedTypeNode*>(varDecl->typeDef))
                    typePart = ", type:" + nt->typeName;
            }
            return "VarDecl('" + nm + "')" + typePart + compactMetaSym(info);
        }
        return "VarDecl" + compactMetaNode(node);
    }
    if (auto* constDecl = dynamic_cast<ConstDeclNode*>(node)) {
        SymbolInfo* info = const_cast<SymbolTable&>(symbolTable).lookup(constDecl->constName);
        return "ConstDecl('" + constDecl->constName + "')" + compactMetaSym(info);
    }
    if (auto* typeDecl = dynamic_cast<TypeDeclNode*>(node)) {
        SymbolInfo* info = const_cast<SymbolTable&>(symbolTable).lookup(typeDecl->typeName);
        return "TypeDecl('" + typeDecl->typeName + "')" + compactMetaSym(info);
    }
    if (auto* subprog = dynamic_cast<SubprogramDeclNode*>(node)) {
        string kind = subprog->isFunction ? "FunctionDecl" : "ProcedureDecl";
        SymbolInfo* info = const_cast<SymbolTable&>(symbolTable).lookup(subprog->subprogramName);
        int sb = info ? info->blockIndex : -1;
        return kind + "('" + subprog->subprogramName + "')" + compactMetaSym(info, sb);
    }

    // Statement
    if (auto* assign = dynamic_cast<AssignNode*>(node)) {
        string targetStr = assign->target ? astToString(assign->target) : "?";
        string valueStr  = assign->value  ? astToString(assign->value)  : "?";
        return "Assign('" + targetStr + "' := " + valueStr + ")"
               + compactMetaNode(node);
    }
    if (dynamic_cast<IfNode*>(node))    return "IfStatement"  + compactMetaNode(node);
    if (dynamic_cast<CaseNode*>(node))  return "CaseStatement"+ compactMetaNode(node);
    if (dynamic_cast<CaseBlockNode*>(node)) return "CaseBlock"+ compactMetaNode(node);
    if (dynamic_cast<WhileNode*>(node)) return "WhileLoop"    + compactMetaNode(node);
    if (dynamic_cast<RepeatNode*>(node))return "RepeatUntil"  + compactMetaNode(node);
    if (auto* forNode = dynamic_cast<ForNode*>(node)) {
        string dir = forNode->isDownto ? "downto" : "to";
        return "ForLoop('" + forNode->iteratorName + "', " + dir + ")"
               + compactMetaNode(node);
    }
    if (auto* procCall = dynamic_cast<ProcCallNode*>(node)) {
        SymbolInfo* info = const_cast<SymbolTable&>(symbolTable).lookup(procCall->procName);
        bool pred = info && info->declLine == 0;
        string base = "writeln(...)"; 
        if (!pred) base = "ProcCall('" + procCall->procName + "')";
        string meta = compactMetaSym(info);
        if (pred) meta += ", predefined";  
        return (pred ? procCall->procName + "(...)" : "ProcCall('" + procCall->procName + "')")
               + compactMetaNode(node);
    }

    // Ekspresi
    if (auto* varNode = dynamic_cast<VarNode*>(node)) {
        SymbolInfo* info = const_cast<SymbolTable&>(symbolTable).lookup(varNode->name);
        string meta = info ? compactMetaSym(info) : compactMetaNode(node);
        return "'" + varNode->name + "'" + meta;
    }
    if (auto* binOp = dynamic_cast<BinOpNode*>(node))
        return "BinOp '" + binOp->op + "'" + compactMetaNode(node);
    if (auto* unOp = dynamic_cast<UnaryOpNode*>(node))
        return "UnaryOp '" + unOp->op + "'" + compactMetaNode(node);
    if (auto* num = dynamic_cast<NumberNode*>(node))
        return num->value + compactMetaNode(node);
    if (auto* str = dynamic_cast<StringNode*>(node))
        return "'" + str->value + "'" + compactMetaNode(node);
    if (auto* chr = dynamic_cast<CharNode*>(node))
        return "'" + chr->value + "'" + compactMetaNode(node);
    if (dynamic_cast<ArrayAccessNode*>(node))  return "ArrayAccess"  + compactMetaNode(node);
    if (auto* recAccess = dynamic_cast<RecordAccessNode*>(node))
        return "RecordAccess('" + recAccess->fieldName + "')" + compactMetaNode(node);

    if (auto* namedType = dynamic_cast<NamedTypeNode*>(node))
        return "Type('" + namedType->typeName + "')" + compactMetaNode(node);
    if (dynamic_cast<ArrayTypeNode*>(node))  return "ArrayType"  + compactMetaNode(node);
    if (dynamic_cast<RecordTypeNode*>(node)) return "RecordType" + compactMetaNode(node);
    if (auto* rangeNode = dynamic_cast<RangeNode*>(node))
        return "Range(" + astToString(rangeNode->lowerBound) + ".." + astToString(rangeNode->upperBound) + ")" + compactMetaNode(node);
    if (auto* enumNode = dynamic_cast<EnumNode*>(node)) {
        string items;
        for (size_t i = 0; i < enumNode->identifiers.size(); ++i) {
            if (i > 0) items += ", ";
            items += enumNode->identifiers[i];
        }
        return "Enum(" + items + ")" + compactMetaNode(node);
    }

    return "ASTNode" + compactMetaNode(node);
}

void writeDecoratedASTRecursive(ofstream& out, ASTNode* node, const SymbolTable& symbolTable, int blockIndex, const string& prefix = "", bool isLast = true, bool isRoot = true) {
    if (!node) return;

    if (!isRoot) {
        out << prefix << (isLast ? "└─ " : "├─ ");
    }

    string newPrefix = prefix + (isRoot ? "" : (isLast ? "   " : "│  "));

    auto rec = [&](ASTNode* child, int bi, bool last) {
        writeDecoratedASTRecursive(out, child, symbolTable, bi, newPrefix, last, false);
    };

    // Program
    if (auto* progNode = dynamic_cast<ProgramNode*>(node)) {
        SymbolInfo* info = const_cast<SymbolTable&>(symbolTable).lookup(progNode->getProgramName());
        int pb = info ? info->blockIndex : blockIndex;
        out << nodeLabel(node, symbolTable, pb) << "\n";

        bool hasBlock = (progNode->mainBlock != nullptr);
        if (!progNode->declarations.empty()) {
            out << newPrefix << (hasBlock ? "├─ " : "└─ ") << "Declarations\n";
            string declPfx = newPrefix + (hasBlock ? "│  " : "   ");
            for (size_t i = 0; i < progNode->declarations.size(); ++i) {
                bool last = (i == progNode->declarations.size() - 1);
                writeDecoratedASTRecursive(out, progNode->declarations[i], symbolTable, -1, declPfx, last, false);
            }
        }
        if (progNode->mainBlock)
            rec(progNode->mainBlock, pb, true);
        return;
    }

    // Block / Compound
    if (auto* compound = dynamic_cast<CompoundNode*>(node)) {
        out << nodeLabel(node, symbolTable, blockIndex) << "\n";
        for (size_t i = 0; i < compound->statements.size(); ++i)
            rec(compound->statements[i], -1, i == compound->statements.size() - 1);
        return;
    }

    // Assign
    if (auto* assign = dynamic_cast<AssignNode*>(node)) {
        out << nodeLabel(node, symbolTable, -1) << "\n";
        vector<ASTNode*> ch;
        if (assign->target) ch.push_back(assign->target);
        if (assign->value)  ch.push_back(assign->value);
        for (size_t i = 0; i < ch.size(); ++i) rec(ch[i], -1, i == ch.size()-1);
        return;
    }

    // BinOp
    if (auto* binOp = dynamic_cast<BinOpNode*>(node)) {
        out << nodeLabel(node, symbolTable, -1) << "\n";
        vector<ASTNode*> ch;
        if (binOp->left)  ch.push_back(binOp->left);
        if (binOp->right) ch.push_back(binOp->right);
        for (size_t i = 0; i < ch.size(); ++i) rec(ch[i], -1, i == ch.size()-1);
        return;
    }

    // UnaryOp
    if (auto* unOp = dynamic_cast<UnaryOpNode*>(node)) {
        out << nodeLabel(node, symbolTable, -1) << "\n";
        if (unOp->operand) rec(unOp->operand, -1, true);
        return;
    }

    // If
    if (auto* ifNode = dynamic_cast<IfNode*>(node)) {
        out << nodeLabel(node, symbolTable, -1) << "\n";
        vector<ASTNode*> ch;
        if (ifNode->condition) ch.push_back(ifNode->condition);
        if (ifNode->thenBlock)  ch.push_back(ifNode->thenBlock);
        if (ifNode->elseBlock)  ch.push_back(ifNode->elseBlock);
        for (size_t i = 0; i < ch.size(); ++i) rec(ch[i], -1, i == ch.size()-1);
        return;
    }

    // While
    if (auto* whileNode = dynamic_cast<WhileNode*>(node)) {
        out << nodeLabel(node, symbolTable, -1) << "\n";
        vector<ASTNode*> ch;
        if (whileNode->condition) ch.push_back(whileNode->condition);
        if (whileNode->loopBlock) ch.push_back(whileNode->loopBlock);
        for (size_t i = 0; i < ch.size(); ++i) rec(ch[i], -1, i == ch.size()-1);
        return;
    }

    // For
    if (auto* forNode = dynamic_cast<ForNode*>(node)) {
        out << nodeLabel(node, symbolTable, -1) << "\n";
        vector<ASTNode*> ch;
        if (forNode->startValue) ch.push_back(forNode->startValue);
        if (forNode->endValue)   ch.push_back(forNode->endValue);
        if (forNode->loopBlock)  ch.push_back(forNode->loopBlock);
        for (size_t i = 0; i < ch.size(); ++i) rec(ch[i], -1, i == ch.size()-1);
        return;
    }

    // Subprogram
    if (auto* subprog = dynamic_cast<SubprogramDeclNode*>(node)) {
        out << nodeLabel(node, symbolTable, -1) << "\n";
        vector<ASTNode*> ch;
        for (auto* p : subprog->parameters)   if (p) ch.push_back(p);
        for (auto* d : subprog->declarations) if (d) ch.push_back(d);
        if (subprog->body) ch.push_back(subprog->body);
        SymbolInfo* info = const_cast<SymbolTable&>(symbolTable).lookup(subprog->subprogramName);
        int subBlock = info ? info->blockIndex : -1;
        for (size_t i = 0; i < ch.size(); ++i) {
            int bi = (ch[i] == subprog->body) ? subBlock : -1;
            writeDecoratedASTRecursive(out, ch[i], symbolTable, bi, newPrefix, i == ch.size()-1, false);
        }
        return;
    }

    // ProcCall
    if (auto* procCall = dynamic_cast<ProcCallNode*>(node)) {
        out << nodeLabel(node, symbolTable, -1) << "\n";
        const auto& args = procCall->getArguments();
        for (size_t i = 0; i < args.size(); ++i)
            rec(args[i], -1, i == args.size()-1);
        return;
    }

    // Leaf / fallthrough
    out << nodeLabel(node, symbolTable, -1) << "\n";
}

void writeSemanticResult(const string& filepath, ASTNode* root, const class SemanticAnalyzer& analyzer, bool hasErrors) {
    ofstream outFile(filepath);
    if (!outFile.is_open()) {
        throw runtime_error("Gagal membuka file untuk menulis semantic result: " + filepath);
    }

    const auto& errorReporter = analyzer.getErrorReporter();
    const auto& symbolTable = analyzer.getSymbolTable();
    const auto& allSymbols = symbolTable.allSymbols();
    const auto& allBlocks = symbolTable.getAllBlocks();
    const auto& allArrays = symbolTable.getAllArrays();

    // SEMANTIC ERROR REPORT
    outFile << "SEMANTIC ERROR REPORT\n";
    if (!hasErrors && !errorReporter.hasErrors()) {
        outFile << "NO SEMANTIC ERRORS (Program is semantically valid)\n";
    } else {
        outFile << "SEMANTIC ERRORS FOUND:\n";
        const auto& errors = errorReporter.getErrors();
        if (!errors.empty()) {
            for (const string& err : errors) {
                outFile << "  " << err << "\n";
            }
        }
    }
    outFile << "\n";

    // SYMBOL TABLE (TAB)
        outFile << "TAB (hanya sebagian yang relevan):\n";
        outFile << left << setw(5) << "idx"
            << setw(18) << "id"
            << setw(12) << "obj"
            << setw(12) << "type"
            << setw(6)  << "ref"
            << setw(6)  << "nrm"
            << setw(6)  << "lev"
            << setw(6)  << "adr"
            << setw(6)  << "link" << "\n";
        outFile << string(80, '-') << "\n";

    std::vector<const SymbolInfo*> tabRows;
    std::vector<const SymbolInfo*> builtinRows;
    for (const auto& sym : allSymbols) {
        if (!shouldPrintTabSymbol(sym)) {
            continue;
        }
        if (isPredefinedSymbol(sym)) {
            builtinRows.push_back(&sym);
        } else {
            tabRows.push_back(&sym);
        }
    }
    tabRows.insert(tabRows.end(), builtinRows.begin(), builtinRows.end());

    int displayIndex = 33;
    for (const SymbolInfo* sym : tabRows) {
        if (isPredefinedSymbol(*sym)) {
            outFile << left << setw(5)  << displayIndex++
                    << setw(18) << sym->name
                    << setw(12) << symbolKindToString(sym->kind)
                    << setw(12) << "..."
                    << setw(6)  << "..."
                    << setw(6)  << "..."
                    << setw(6)  << "..."
                    << setw(6)  << "..."
                    << setw(6)  << "..."
                    << "\t(predefined)" << "\n";
        } else {
            outFile << left << setw(5)  << displayIndex++
                    << setw(18) << sym->name
                    << setw(12) << tabObjectLabel(*sym)
                    << setw(12) << exprTypeToString(sym->type)
                    << setw(6)  << (sym->tabIndex)
                    << setw(6)  << 1
                    << setw(6)  << sym->level
                    << setw(6)  << sym->declLine
                    << setw(6)  << 0
                    << "\n";
        }
    }
    outFile << "\n";

    // BLOCK TABLE (BTAB)
    outFile << "btab:\n";
    outFile << left << setw(6) << "idx"
            << setw(8) << "last"
            << setw(8) << "lpar"
            << setw(8) << "psze"
            << setw(8) << "vsze" << "\n";
    outFile << string(50, '-') << "\n";

    for (const auto& block : allBlocks) {
        int last = 0;
        int lpar = 0;
        int psze = 0;
        int vsze = 0;

        for (const auto& sym : allSymbols) {
            if (sym.blockIndex != block.blockIndex) continue;
            if (sym.tabIndex > last) last = sym.tabIndex;
            if (sym.kind == SymbolKind::PARAMETER) {
                if (sym.tabIndex > lpar) lpar = sym.tabIndex;
                ++psze;
            } else if (sym.kind == SymbolKind::VARIABLE) {
                ++vsze;
            }
        }

        outFile << left << setw(6) << block.blockIndex
                << setw(8) << last
                << setw(8) << lpar
                << setw(8) << psze
                << setw(8) << vsze << "\n";
    }
    outFile << "\n";

    // ARRAY TABLE (ATAB)
    outFile << "ATAB:\n";
    if (allArrays.empty()) {
        outFile << "  (No arrays declared)\n";
    } else {
        outFile << left << setw(8)  << "arrays"
            << setw(8)  << "xtyp"
            << setw(8)  << "etyp"
            << setw(8)  << "eref"
            << setw(8)  << "low"
            << setw(8)  << "high"
            << setw(8)  << "elsz"
            << setw(8)  << "size" << "\n";
        outFile << string(64, '-') << "\n";
        for (const auto& arr : allArrays) {
            int xtyp = typeCodeForNode(symbolTable, arr.indexTypeNode);
            int etyp = typeCodeForNode(symbolTable, arr.elementTypeNode);
            int eref = arrayDetailRef(symbolTable, arr.elementTypeNode);
            int elsz = typeSizeForNode(symbolTable, arr.elementTypeNode);
            int totalSize = 0;
            if (arr.hasStaticBounds && arr.upperBound >= arr.lowerBound) {
                totalSize = (arr.upperBound - arr.lowerBound + 1) * elsz;
            } else {
                totalSize = elsz;
            }

            outFile << left << setw(8)  << (arr.arrayIndex + 1)
                    << setw(8)  << xtyp
                    << setw(8)  << etyp
                    << setw(8)  << eref
                    << setw(8)  << arr.lowerBound
                    << setw(8)  << arr.upperBound
                    << setw(8)  << elsz
                    << setw(8)  << totalSize << "\n";
        }
    }
    outFile << "\n";

    outFile << "DECORATED AST: \n";
    if (root) {
        writeDecoratedASTRecursive(outFile, root, symbolTable, -1);
    } else {
        outFile << "(AST kosong)\n";
    }
    outFile << "\nSemantic analysis completed\n";
    outFile.close();
}