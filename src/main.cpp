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

    size_t dotIndex = filename.find_last_of('.');
    string baseName = (dotIndex != string::npos) ? filename.substr(0, dotIndex) : filename;
    string extension = (dotIndex != string::npos) ? filename.substr(dotIndex) : ".txt";
    
    string outputFilePath = "test/milestone-1/" + baseName + "Lex" + extension;

    ofstream outputFile(outputFilePath);
    if (!outputFile.is_open()) {
        cerr << "Gagal membuat file output: " << outputFilePath << endl;
        return 1;
    }

    for (const Token& token : tokens) {
        string typeStr = typeToString(token.type);
        
        if (token.type == TokenType::IDENTIFIER || token.type == TokenType::INTCON || 
            token.type == TokenType::REALCON || token.type == TokenType::STRING || token.type == TokenType::CHARCON) {
            
            outputFile << typeStr << " (" << token.value << ")" << endl;
            
        } else {
            outputFile << typeStr << endl;
        }
    }

    outputFile.close();
    cout << "\nBerhasil! Daftar token telah disimpan dalam " << outputFilePath << endl;

    return 0;
}