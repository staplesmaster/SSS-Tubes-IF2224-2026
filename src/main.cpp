#include <iostream>
#include <vector>
#include "Lexer.hpp"
#include "Token.hpp"
#include "Reader.hpp"
#include "Writer.hpp"

using namespace std;

int main() {
    string filename;
    cout << "Masukkan nama file (contoh: input.txt): ";
    cin >> filename;

    // Input file
    string inputFilePath = "test/milestone-1/" + filename;
    string sourceCode;
    try {
        sourceCode = readFile(inputFilePath);
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    // Lexical analysis
    Lexer lexer(sourceCode);
    vector<Token> tokens;
    try {
        tokens = lexer.tokenize(); 
    } catch (const exception& e) {
        cerr << "Terjadi error saat Lexing: " << e.what() << endl;
        return 1;
    }

    // Write output file
    int lastIndex = filename.find_last_of('.');
    string baseName = (lastIndex != int(string::npos)) ? filename.substr(0, lastIndex) : filename;
    string extension = (lastIndex != int(string::npos)) ? filename.substr(lastIndex) : ".txt";
    string outputFilePath = "test/milestone-1/" + baseName + "-Result" + extension;

    try {
        writeTokens(outputFilePath, tokens, sourceCode);
        cout << "\nBerhasil! Daftar token telah disimpan dalam " << outputFilePath << endl;
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}