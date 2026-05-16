#include "Writer.hpp"
#include <fstream>
#include <stdexcept>

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
        // -------------------------------------------------------------
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
        outFile << "=== DAFTAR SYNTAX ERROR ===\n";
        for (const string& err : errors) {
            outFile << err << "\n";
        }
        outFile << "\n";
    } else {
        outFile << "=== TIDAK ADA SYNTAX ERROR (PROGRAM VALID) ===\n\n";
    }

    outFile << "=== HASIL PARSE TREE ===\n";
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

// Helper: Cetak struktur vertikal untuk Blok, Statement, dan Deklarasi
void writeASTRecursive(ofstream& out, ASTNode* node, const string& prefix = "", bool isLast = true, bool isRoot = true) {
    if (node == nullptr) return;

    if (!isRoot) {
        out << prefix;
        out << (isLast ? "└── " : "├── ");
    }

    string newPrefix = prefix + (isRoot ? "" : (isLast ? "    " : "│   "));

    // --- PROGRAM & COMPOUND ---
    if (auto* progNode = dynamic_cast<ProgramNode*>(node)) {
        out << "ProgramNode(name: '" << progNode->getProgramName() << "')\n";
        
        // Cabang Declarations
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
        writeASTRecursive(out, progNode->mainBlock, blockPrefix, true, false);
    }
    else if (auto* compNode = dynamic_cast<CompoundNode*>(node)) {
        out << "CompoundStatement\n";
        for (size_t i = 0; i < compNode->statements.size(); ++i) {
            bool childLast = (i == compNode->statements.size() - 1);
            writeASTRecursive(out, compNode->statements[i], newPrefix, childLast, false);
        }
    }

    // --- DECLARATIONS ---
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

    // --- STATEMENTS ---
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
    
    // Fallback jika node tidak dikenali secara spesifik untuk layout vertikal
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

    outFile << "=== HASIL ABSTRACT SYNTAX TREE (AST) ===\n";
    if (root != nullptr) {
        writeASTRecursive(outFile, root);
    } else {
        outFile << "AST Kosong (Terdapat Syntax Error sehingga AST tidak dibentuk).\n";
    }

    outFile.close();
}