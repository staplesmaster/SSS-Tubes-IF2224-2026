#include "ParseNode.hpp"
#include <iostream>

using namespace std;

ParseNode::ParseNode(const string& name) : name(name), isTerminal(false) {
    token.type = UNKNOWN;
}

ParseNode::ParseNode(const Token& token) : name(""), token(token), isTerminal(true) {}

ParseNode::~ParseNode() {
    for (ParseNode* child : children) {
        delete child;
    }
}

void ParseNode::addChild(ParseNode* child) {
    if (child != nullptr) {
        children.push_back(child);
    }
}

string ParseNode::getName() const { 
    return name; 
}

Token ParseNode::getToken() const { 
    return token; 
}

bool ParseNode::getIsTerminal() const { 
    return isTerminal; 
}

const vector<ParseNode*>& ParseNode::getChildren() const { 
    return children; 
}

void ParseNode::printTree(const string& prefix, bool isLast, bool isRoot) const {
   
    if (!isRoot) {
        cout << prefix;
        cout << (isLast ? "└── " : "├── "); 
    }

    if (!isTerminal) {
        cout << name << "\n";
    } else {
        string typeStr = typeToString(token.type);
        if (token.type == TokenType::IDENTIFIER || token.type == TokenType::INTCON || 
            token.type == TokenType::REALCON || token.type == TokenType::STRING || 
            token.type == TokenType::CHARCON || token.type == TokenType::UNKNOWN) {
            cout << typeStr << "(" << token.value << ")\n";
        } else {
            cout << typeStr << "\n";
        }
    }

    for (size_t i = 0; i < children.size(); ++i) {
        string newPrefix = prefix;
        if (!isRoot) {
            newPrefix += (isLast ? "    " : "│   "); 
        }
        
        bool childIsLast = (i == children.size() - 1);
        
        children[i]->printTree(newPrefix, childIsLast, false);
    }
}