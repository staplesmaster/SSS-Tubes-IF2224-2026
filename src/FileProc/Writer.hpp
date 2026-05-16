#ifndef WRITER_HPP
#define WRITER_HPP

#include <string>
#include <vector>
#include "Token.hpp"
#include "ParseNode.hpp"
#include "ASTNode.hpp"
#include "SymbolInfo.hpp"
using namespace std;

void writeTokens(const string& filepath, const vector<Token>& tokens, const string& sourceCode);

void writeParseResult(const string& filepath, ParseNode* root, const vector<string>& errors);

void writeASTResult(const string& filepath, ASTNode* root);

void writeSemanticResult(const string& filepath, ASTNode* root, const class SemanticAnalyzer& analyzer, bool hasErrors);
string compactMetaNode(const ASTNode* node, int blockIndex = -1, bool predefined = false);
string compactMetaSym(const SymbolInfo* info, int blockIndex = -1);

#endif