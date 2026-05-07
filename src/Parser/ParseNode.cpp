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