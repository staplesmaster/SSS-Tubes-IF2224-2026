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
#include "CodeGen/CodeGenerator.hpp"
#include "Interpreter/VM.hpp"
#include <fstream>

using namespace std;

int main() {
    string filename;
    cout << "Masukkan nama file (contoh: input.txt): ";
    cin >> filename;

    // Input file
    string inputFilePath = "test/milestone-4/" + filename;
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
    string tokenOutputPath = "test/milestone-4/" + baseName + "-Result-Token" + extension;
    string parseOutputPath = "test/milestone-4/" + baseName + "-Result-Parse" + extension;
    string astOutputPath   = "test/milestone-4/" + baseName + "-Result-AST" + extension;
    string semanticOutputPath = "test/milestone-4/" + baseName + "-Result-Semantic" + extension;

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

        if (!semanticHasErrors && astRoot) {
            try {
                SymbolTable& symtab = analyzer.getSymbolTable();
                CodeGenerator codegen(symtab);
                codegen.generate(astRoot);

                const auto& program = codegen.getInstructions();
                std::string dumpPath = "test/milestone-4/" + baseName + "-Result-PCode" + extension;
                std::ofstream ofs(dumpPath);
                if (ofs) ofs << codegen.dump();
                cout << "File P-Code: " << dumpPath << "\n";

                VM vm(program, cin, cout);
                vm.run();
            } catch (const std::exception& e) {
                cerr << "Runtime error: " << e.what() << endl;
            }
        }

    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    delete parseTreeRoot;
    if (astRoot) delete astRoot;

    return 0;
}