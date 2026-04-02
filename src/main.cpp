#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Lexer.hpp"
#include "Token.hpp"

using namespace std;

int main() {
    string filename;
    cout << "Masukkan nama file (contoh: input.txt): ";
    cin >> filename;

    string inputFilePath = "test/milestone-1/" + filename;

    ifstream inputFile(inputFilePath);
    if (!inputFile.is_open()) {
        cerr << "Gagal membuka file input: " << inputFilePath << endl;
        return 1; 
    }

    stringstream buffer;
    buffer << inputFile.rdbuf();
    string sourceCode = buffer.str();
    inputFile.close();

    Lexer lexer(sourceCode);
    vector<Token> tokens;
    
    try {
        tokens = lexer.tokenize(); 
    } catch (const exception& e) {
        cerr << "Terjadi error saat Lexing: " << e.what() << endl;
        return 1;
    }

    int lastIndex = filename.find_last_of('.');
    string baseName = (lastIndex != int(string::npos)) ? filename.substr(0, lastIndex) : filename;
    string extension = (lastIndex != int(string::npos)) ? filename.substr(lastIndex) : ".txt";
    
    string outputFilePath = "test/milestone-1/" + baseName + "-Result" + extension;

    ofstream outputFile(outputFilePath);
    if (!outputFile.is_open()) {
        cerr << "Gagal membuat file output: " << outputFilePath << endl;
        return 1;
    }

    int previousEnd = 0;
    bool firstToken = true;

    for (Token& token : tokens) {
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
            outputFile << endl;
        }

        string typeStr = typeToString(token.type);
        string lexeme = sourceCode.substr(token.start, token.end - token.start);
        
        if (token.type == TokenType::IDENTIFIER || token.type == TokenType::INTCON || 
            token.type == TokenType::REALCON || token.type == TokenType::STRING || token.type == TokenType::CHARCON) {
            
            outputFile << typeStr << " (" << lexeme << ")" << endl;
            
        } else {
            outputFile << typeStr << endl;
        }

        previousEnd = token.end;
        firstToken = false;
    }

    outputFile.close();
    cout << "\nBerhasil! Daftar token telah disimpan dalam " << outputFilePath << endl;

    return 0;
}