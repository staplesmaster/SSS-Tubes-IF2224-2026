#include <iostream>
#include <vector>
#include <string>
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

static bool runPipeline(const string& sourceCode,ParseNode*& outParseRoot,ASTNode*& outAstRoot,SemanticAnalyzer& outAnalyzer,vector<string>& outSyntaxErrors,bool& outSemanticHasErrors
) {
    Lexer lexer(sourceCode);
    vector<Token> tokens;
    try {
        tokens = lexer.tokenize();
    } catch (const exception& e) {
        cerr << "Terjadi error saat Lexing: " << e.what() << endl;
        return false;
    }

    Parser parser(tokens, sourceCode);
    outParseRoot = parser.parse();
    outSyntaxErrors = parser.getErrors();

    ASTConverter astConverter;
    outAstRoot = nullptr;
    if (outParseRoot != nullptr && outSyntaxErrors.empty()) {
        outAstRoot = astConverter.build(outParseRoot);
    }

    outSemanticHasErrors = false;
    if (outAstRoot != nullptr && outSyntaxErrors.empty()) {
        outAnalyzer.analyze(outAstRoot);
        outSemanticHasErrors = outAnalyzer.getErrorReporter().hasErrors();
    }

    return true;
}

int main() {
    const string EXEC_MARKER = "-Result-AST";

    while (true) {
        string filename;
        cout << "Masukkan nama file (contoh: testCase1.txt atau testCase1-Result-AST.txt): ";
        if (!(cin >> filename)) break;

        bool isExecMode = (filename.find(EXEC_MARKER) != string::npos);

        if (isExecMode) {
            int lastIndex = filename.find_last_of('.');
            string baseName = (lastIndex != int(string::npos)) ? filename.substr(0, lastIndex) : filename;
            string extension = (lastIndex != int(string::npos)) ? filename.substr(lastIndex) : ".txt";

            size_t markerPos = baseName.find(EXEC_MARKER);
            string sourceBase = baseName.substr(0, markerPos);

            string astFilePath = "test/milestone-4/" + filename;
            {
                ifstream astCheck(astFilePath);
                if (!astCheck.is_open()) {
                    cerr << "File AST '" << astFilePath << "' tidak ditemukan.\n";
                    cerr << "Jalankan preprocessing terlebih dahulu dengan menginput:\n";
                    cerr << "  " << sourceBase << extension << "\n";
                    continue;
                }
            }

            string sourceFilePath = "test/milestone-4/" + sourceBase + extension;

            string sourceCode;
            try {
                sourceCode = readFile(sourceFilePath);
            } catch (const exception& e) {
                cerr << "Gagal membaca source file '" << sourceFilePath << "': " << e.what() << endl;
                cerr << "Pastikan file source '" << sourceBase << extension << "' tersedia di test/milestone-4/" << endl;
                continue;
            }

            ParseNode* parseRoot = nullptr;
            ASTNode*   astRoot   = nullptr;
            SemanticAnalyzer analyzer;
            vector<string> syntaxErrors;
            bool semanticHasErrors = false;

            if (!runPipeline(sourceCode, parseRoot, astRoot, analyzer, syntaxErrors, semanticHasErrors)) {
                continue;
            }

            if (!syntaxErrors.empty() || astRoot == nullptr) {
                cout << "\nSource code tidak valid, AST tidak terbuat dengan baik\n";
                cout << "untuk menjalankan kode.\n";
                if (!syntaxErrors.empty()) {
                    cout << "\nDaftar Syntax Error:\n";
                    for (const string& err : syntaxErrors) {
                        cout << "  " << err << "\n";
                    }
                }
                delete parseRoot;
                if (astRoot) delete astRoot;
                continue;
            }

            if (semanticHasErrors) {
                cout << "\nSource code tidak valid, AST tidak terbuat dengan baik\n";
                cout << "untuk menjalankan kode.\n";
                cout << "(Ditemukan Semantic Error - program tidak dapat dieksekusi)\n";
                const auto& errors = analyzer.getErrorReporter().getErrors();
                if (!errors.empty()) {
                    cout << "\nDaftar Semantic Error:\n";
                    for (const string& err : errors) {
                        cout << "  " << err << "\n";
                    }
                }
                delete parseRoot;
                if (astRoot) delete astRoot;
                continue;
            }

            cout << "\nMenjalankan program dari: " << filename << "\n\n";
            try {
                SymbolTable& symtab = analyzer.getSymbolTable();
                CodeGenerator codegen(symtab);
                codegen.generate(astRoot);

                const auto& program = codegen.getInstructions();

                string pcodePath = "test/milestone-4/" + sourceBase + "-Result-Pcode" + extension;
                ofstream ofs(pcodePath);
                if (ofs) ofs << codegen.dump();
                cout << "File P-Code disimpan: " << pcodePath << "\n";
                cout << "------------------------------------------------\n";
                cout << "Output Program:\n\n";

                VM vm(program, cin, cout);
                vm.run();

                cout << "\n------------------------------------------------\n";
                cout << "Eksekusi selesai.\n";
            } catch (const exception& e) {
                cerr << "Runtime error: " << e.what() << endl;
            }

            delete parseRoot;
            if (astRoot) delete astRoot;

        } else {
            string inputFilePath = "test/milestone-4/" + filename;
            string sourceCode;
            try {
                sourceCode = readFile(inputFilePath);
            } catch (const exception& e) {
                cerr << e.what() << endl;
                continue;
            }

            Lexer lexer(sourceCode);
            vector<Token> tokens;
            try {
                tokens = lexer.tokenize();
            } catch (const exception& e) {
                cerr << "Terjadi error saat Lexing: " << e.what() << endl;
                continue;
            }

            Parser parser(tokens, sourceCode);
            ParseNode* parseTreeRoot = parser.parse();
            vector<string> syntaxErrors = parser.getErrors();

            ASTConverter astConverter;
            ASTNode* astRoot = nullptr;
            if (parseTreeRoot != nullptr && syntaxErrors.empty()) {
                astRoot = astConverter.build(parseTreeRoot);
            }

            SemanticAnalyzer analyzer;
            bool semanticHasErrors = false;
            if (astRoot != nullptr && syntaxErrors.empty()) {
                analyzer.analyze(astRoot);
                semanticHasErrors = analyzer.getErrorReporter().hasErrors();
            }

            int lastIndex = filename.find_last_of('.');
            string baseName  = (lastIndex != int(string::npos)) ? filename.substr(0, lastIndex) : filename;
            string extension = (lastIndex != int(string::npos)) ? filename.substr(lastIndex) : ".txt";

            string tokenOutputPath    = "test/milestone-4/" + baseName + "-Result-Token"    + extension;
            string parseOutputPath    = "test/milestone-4/" + baseName + "-Result-Parse"    + extension;
            string astOutputPath      = "test/milestone-4/" + baseName + "-Result-AST"      + extension;
            string semanticOutputPath = "test/milestone-4/" + baseName + "-Result-Semantic" + extension;

            try {
                writeTokens(tokenOutputPath, tokens, sourceCode);
                writeParseResult(parseOutputPath, parseTreeRoot, syntaxErrors);
                writeASTResult(astOutputPath, astRoot);
                writeSemanticResult(semanticOutputPath, astRoot, analyzer, semanticHasErrors);

                cout << "\nBerhasil! File hasil analisis telah disimpan:\n";
                cout << "  File Token   : " << tokenOutputPath    << "\n";
                cout << "  File Parse   : " << parseOutputPath    << "\n";
                cout << "  File AST     : " << astOutputPath      << "\n";
                cout << "  File Semantic: " << semanticOutputPath << "\n";
                cout << "\nUntuk menjalankan program, masukkan file AST:\n";
                cout << "  " << baseName << "-Result-AST" << extension << "\n";

            } catch (const exception& e) {
                cerr << e.what() << endl;
            }

            delete parseTreeRoot;
            if (astRoot) delete astRoot;
        }

        cout << "\n";
    }

    return 0;
}