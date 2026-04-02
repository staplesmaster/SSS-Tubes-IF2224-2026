#include "Writer.hpp"
#include <fstream>
#include <stdexcept>

void writeTokens(const string& filepath, const vector<Token>& tokens, const string& sourceCode) {
    ofstream outputFile(filepath);
    if (!outputFile.is_open()) {
        throw runtime_error("Gagal membuat file output: " + filepath);
    }

    int previousEnd = 0;
    bool firstToken = true;

    for (const Token& token : tokens) {
        int newlineCount = 0;
        for (int i = previousEnd; i < token.start && i < (int)sourceCode.size(); ++i) {
            if (sourceCode[i] == '\n') {
                ++newlineCount;
            }
        }

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