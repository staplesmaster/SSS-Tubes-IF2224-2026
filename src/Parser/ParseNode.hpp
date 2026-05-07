#pragma once

#include <vector>
#include <string>
#include "Token.hpp"

using namespace std;

class ParseNode {
private:
    string name;                 // Label Non-Terminal (contoh: "<program>")
    Token token;                 // Token Terminal
    bool isTerminal;
    vector<ParseNode*> children;

public:
    // Constructor Non-Terminal
    ParseNode(const string& name);

    // Constructor Terminal
    ParseNode(const Token& token);

    ~ParseNode();

    void addChild(ParseNode* child);

    string getName() const;
    Token getToken() const;
    bool getIsTerminal() const;
    const vector<ParseNode*>& getChildren() const;
};