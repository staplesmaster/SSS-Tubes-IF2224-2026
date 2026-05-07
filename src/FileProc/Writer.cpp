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