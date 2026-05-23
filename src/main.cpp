#include <iostream>
#include <vector>
#include "Lexer.hpp"
#include "Token.hpp"
#include "Reader.hpp"
#include "Writer.hpp"
#include "Parser.hpp"
#include "ASTNode.hpp"
#include "ASTConverter.hpp"
#include "SemanticAnalyzer.hpp"

using namespace std;

int main() {
    string filename;
    cout << "Masukkan nama file (contoh: input.txt): ";
    cin >> filename;

    // Input file
    string inputFilePath = "test/milestone-3/" + filename;
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

    // Syntax analysis
    Parser parser(tokens, sourceCode);
    ParseNode* parseTreeRoot = parser.parse(); 
    vector<string> syntaxErrors = parser.getErrors();

    // Convert to AST
    ASTConverter astConverter;
    ASTNode* astRoot = nullptr;
    if (parseTreeRoot != nullptr && syntaxErrors.empty()) {
        astRoot = astConverter.build(parseTreeRoot);
    }

    // Semantic Analysis
    SemanticAnalyzer analyzer;
    bool semanticHasErrors = false;
    if (astRoot != nullptr && syntaxErrors.empty()) {
        analyzer.analyze(astRoot);
        semanticHasErrors = analyzer.getErrorReporter().hasErrors();
    }

    // Write output file
    int lastIndex = filename.find_last_of('.');
    string baseName = (lastIndex != int(string::npos)) ? filename.substr(0, lastIndex) : filename;
    string extension = (lastIndex != int(string::npos)) ? filename.substr(lastIndex) : ".txt";
    string tokenOutputPath = "test/milestone-3/" + baseName + "-Result-Token" + extension;
    string parseOutputPath = "test/milestone-3/" + baseName + "-Result-Parse" + extension;
    string astOutputPath   = "test/milestone-3/" + baseName + "-Result-AST" + extension;
    string semanticOutputPath = "test/milestone-3/" + baseName + "-Result-Semantic" + extension;

    try {
        writeTokens(tokenOutputPath, tokens, sourceCode);
        writeParseResult(parseOutputPath, parseTreeRoot, syntaxErrors);
        writeASTResult(astOutputPath, astRoot);
        writeSemanticResult(semanticOutputPath, astRoot, analyzer, semanticHasErrors);
        cout << "\nBerhasil! Daftar token telah disimpan dalam"<< endl;
        cout << "File Token: " << tokenOutputPath << "\n";
        cout << "File Parse: " << parseOutputPath << "\n";
        cout << "File AST: " << astOutputPath << "\n";
        cout << "File Semantic: " << semanticOutputPath << "\n";
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    delete parseTreeRoot;
    if (astRoot) delete astRoot;

    return 0;
}