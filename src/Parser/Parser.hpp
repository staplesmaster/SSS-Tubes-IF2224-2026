#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include "Token.hpp"
#include "ParseNode.hpp"

using namespace std;

class Parser {
private:
    vector<Token> tokens;
    string sourceCode;
    int pos;
    bool hasError = false;
    vector<string> errors;

    Token currentToken() const;
    Token peek(int offset) const; 
    bool isAtEnd() const;
    void advance();
    Token match(TokenType expected);

    ParseNode* parseProgram();
    ParseNode* parseProgramHeader();
    ParseNode* parseBlock();

    ParseNode* parseDeclarationPart();
    ParseNode* parseConstDeclaration();
    ParseNode* parseConstant();
    ParseNode* parseTypeDeclaration();
    ParseNode* parseVarDeclaration();
    ParseNode* parseIdentifierList();
    ParseNode* parseSubprogramDeclaration();
    ParseNode* parseProcedureDeclaration();
    ParseNode* parseFunctionDeclaration();
    ParseNode* parseFormalParameterList(); 
    ParseNode* parseParameterGroup();

    ParseNode* parseType();
    ParseNode* parseArrayType();
    ParseNode* parseRange();
    ParseNode* parseEnumerated();
    ParseNode* parseRecordType();
    ParseNode* parseFieldList();
    ParseNode* parseFieldPart();


    ParseNode* parseCompoundStatement();
    ParseNode* parseStatementList();
    ParseNode* parseStatement();
    ParseNode* parseVariable();
    ParseNode* parseComponentVariable();
    ParseNode* parseIndexList();
    ParseNode* parseAssignmentStatement();
    ParseNode* parseIfStatement();
    ParseNode* parseCaseStatement();
    ParseNode* parseCaseBlock();
    ParseNode* parseWhileStatement();
    ParseNode* parseRepeatStatement();
    ParseNode* parseForStatement();
    ParseNode* parseProcedureFunctionCall();
    ParseNode* parseParameterList();

    // 1 orang
    ParseNode* parseExpression();
    ParseNode* parseSimpleExpression();
    ParseNode* parseTerm();
    ParseNode* parseFactor();
    ParseNode* parseRelationalOperator();
    ParseNode* parseAdditiveOperator();
    ParseNode* parseMultiplicativeOperator();

public:
    Parser(const vector<Token>& tokens, const string& sourceCode);

    ParseNode* parse();
    
    const vector<string>& getErrors() const;
};