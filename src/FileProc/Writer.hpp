#ifndef WRITER_HPP
#define WRITER_HPP

#include <string>
#include <vector>
#include "Token.hpp"

using namespace std;

void writeTokens(const string& filepath, const vector<Token>& tokens, const string& sourceCode);

#endif