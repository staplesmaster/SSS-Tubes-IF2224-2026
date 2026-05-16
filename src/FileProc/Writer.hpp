#ifndef WRITER_HPP
#define WRITER_HPP

#include <string>
#include <vector>
#include "Token.hpp"
#include "ParseNode.hpp"
#include "ASTNode.hpp"

using namespace std;

void writeTokens(const string& filepath, const vector<Token>& tokens, const string& sourceCode);

void writeParseResult(const string& filepath, ParseNode* root, const vector<string>& errors);

void writeASTResult(const string& filepath, ASTNode* root);

#endif