#pragma once
#include <vector>
#include <unordered_map>
#include <iostream>
#include "Token.hpp"

class Lexer {
private : 
    enum class State {
        START,
        IDENT,
        INT,
        REAL,
        STRING,
    };

    string line; 

    int pos;

    char peek(int offset);
    char adv ();
    bool isEnd();

    void skip();

    bool isAlphabet(char c);
    bool isDigit(char c);
    bool isAlphanumeric(char c);
    bool isSpace(char c);

    string toLower(const string& str);

    Token Lexer::handleSymbol();
    
    Token nextToken();

public: 
    Lexer(const string& input);

    vector<Token> tokenize();
};