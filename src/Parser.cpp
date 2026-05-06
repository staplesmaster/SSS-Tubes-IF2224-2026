#include "Parser.hpp"
#include <iostream>

using namespace std;

Parser::Parser(const vector<Token>& tokens): tokens(tokens), pos(0){}

Token Parser::currentToken() const {
    if (isAtEnd()) return tokens.back();
    return tokens[pos];
}

Token Parser::peek(int offset) const{
    if (pos + offset >= int(tokens.size())) return tokens.back(); 
    return tokens[pos + offset];
}

bool Parser::isAtEnd() const {
    return pos >= int(tokens.size());
}

void Parser::advance(){
    if(!isAtEnd()) pos++;
}

Token Parser::match(TokenType expected){
    if (!isAtEnd() && currentToken().type == expected){
        Token t = currentToken();
        advance();
        return t; 
    }
    // Masuk Panic Mode
    string errLex = isAtEnd()? "EOF" : currentToken().value; 
    cout << "[SYNTAX ERROR] Mengharapkan '" << typeToString(expected) 
         << "' tetapi menemukan '" << errLex << "' pada posisi " << pos << "!\n";

    Token errorToken; 
    errorToken.type = UNKNOWN;
    errorToken.value = "MISSING_" + typeToString(expected);
    return errorToken;
}

// declaration-part -> (const)* + (type)* + (var)* + (subprogram)*
ParseNode* Parser::parseDeclarationPart() {
    ParseNode* node = new ParseNode( "<declaration-part>" );

    while (!isAtEnd() && currentToken().type == TokenType::CONST) {
        node->addChild( parseConstDeclaration() );
    }
    while (!isAtEnd() && currentToken().type == TokenType::TYPE) {
        node->addChild( parseTypeDeclaration() );
    }
    while (!isAtEnd() && currentToken().type == TokenType::VAR) {
        node->addChild( parseVarDeclaration() );
    }
    while (!isAtEnd() && (currentToken().type == TokenType::PROCEDURE ||
        currentToken().type == TokenType::FUNCTION)) {
        node->addChild( parseSubprogramDeclaration() );
    }

    return node;
}

// const-declaration -> constsy + (ident + eql + constant + semicolon)+
ParseNode* Parser::parseConstDeclaration() {
    ParseNode* node = new ParseNode( "<const-declaration>" );
    node->addChild( new ParseNode( match( TokenType::CONST ) ) );

    do {
        node->addChild( new ParseNode( match( TokenType::IDENTIFIER ) ) );
        node->addChild( new ParseNode( match( TokenType::EQL ) ) ); // Sesuai contoh grammar "INT==100"
        node->addChild( parseConstant() );
        node->addChild( new ParseNode( match( TokenType::SEMICOLON ) ) );
    } while (!isAtEnd() && currentToken().type == TokenType::IDENTIFIER);

    return node;
}

// constant -> charcon | string | [(plus | minus)? + (ident | intcon | realcon)]
ParseNode* Parser::parseConstant() {
    ParseNode* node = new ParseNode( "<constant>" );
    TokenType t = currentToken().type;

    if (t == TokenType::CHARCON || t == TokenType::STRING) {
        node->addChild( new ParseNode( match( t ) ) );
    }

    else {
        if (t == TokenType::PLUS || t == TokenType::MINUS) {
            node->addChild( new ParseNode( match( t ) ) );
            t = currentToken().type;
        }

        if (t == TokenType::IDENTIFIER || t == TokenType::INTCON || t == TokenType::REALCON) {
            node->addChild( new ParseNode( match( t ) ) );
        }
        else {
            node->addChild( new ParseNode( match( TokenType::UNKNOWN ) ) );
        }
    }
    return node;
}

// type-declaration -> typesy + (ident + eql + type + semicolon)+
ParseNode* Parser::parseTypeDeclaration() {
    ParseNode* node = new ParseNode( "<type-declaration>" );
    node->addChild( new ParseNode( match( TokenType::TYPE ) ) );

    do {
        node->addChild( new ParseNode( match( TokenType::IDENTIFIER ) ) );
        node->addChild( new ParseNode( match( TokenType::EQL ) ) );
        node->addChild( parseType() ); // Akan diimplementasi di Bagian 4
        node->addChild( new ParseNode( match( TokenType::SEMICOLON ) ) );
    } while (!isAtEnd() && currentToken().type == TokenType::IDENTIFIER);

    return node;
}

// var-declaration -> varsy + (identifier-list + colon + type + semicolon)+
ParseNode* Parser::parseVarDeclaration() {
    ParseNode* node = new ParseNode( "<var-declaration>" );
    node->addChild( new ParseNode( match( TokenType::VAR ) ) );

    do {
        node->addChild( parseIdentifierList() );
        node->addChild( new ParseNode( match( TokenType::COLON ) ) );
        node->addChild( parseType() );
        node->addChild( new ParseNode( match( TokenType::SEMICOLON ) ) );
    } while (!isAtEnd() && currentToken().type == TokenType::IDENTIFIER);

    return node;
}

// identifier-list -> ident + (comma + ident)*
ParseNode* Parser::parseIdentifierList() {
    ParseNode* node = new ParseNode( "<identifier-list>" );
    node->addChild( new ParseNode( match( TokenType::IDENTIFIER ) ) );

    while (!isAtEnd() && currentToken().type == TokenType::COMMA) {
        node->addChild( new ParseNode( match( TokenType::COMMA ) ) );
        node->addChild( new ParseNode( match( TokenType::IDENTIFIER ) ) );
    }
    return node;
}

// subprogram-declaration -> procedure-declaration | function-declaration
ParseNode* Parser::parseSubprogramDeclaration() {
    ParseNode* node = new ParseNode( "<subprogram-declaration>" );

    if (currentToken().type == TokenType::PROCEDURE) {
        node->addChild( parseProcedureDeclaration() );
    }
    else if (currentToken().type == TokenType::FUNCTION) {
        node->addChild( parseFunctionDeclaration() );
    }
    else {
        node->addChild( new ParseNode( match( TokenType::UNKNOWN ) ) );
    }
    return node;
}

// procedure-declaration -> proceduresy + ident + (formal-parameter-list)? + semicolon + block + semicolon
ParseNode* Parser::parseProcedureDeclaration() {
    ParseNode* node = new ParseNode( "<procedure-declaration>" );

    node->addChild( new ParseNode( match( TokenType::PROCEDURE ) ) );
    node->addChild( new ParseNode( match( TokenType::IDENTIFIER ) ) );

    if (currentToken().type == TokenType::LPARENT) {
        node->addChild( parseFormalParameterList() );
    }

    node->addChild( new ParseNode( match( TokenType::SEMICOLON ) ) );
    node->addChild( parseBlock() );
    node->addChild( new ParseNode( match( TokenType::SEMICOLON ) ) );

    return node;
}

// function-declaration -> functionsy + ident + (formal-parameter-list)? + colon + ident + semicolon + block + semicolon
ParseNode* Parser::parseFunctionDeclaration() {
    ParseNode* node = new ParseNode( "<function-declaration>" );

    node->addChild( new ParseNode( match( TokenType::FUNCTION ) ) );
    node->addChild( new ParseNode( match( TokenType::IDENTIFIER ) ) );

    if (currentToken().type == TokenType::LPARENT) {
        node->addChild( parseFormalParameterList() );
    }

    node->addChild( new ParseNode( match( TokenType::COLON ) ) );
    node->addChild( new ParseNode( match( TokenType::IDENTIFIER ) ) ); // Tipe kembalian dari fungsi
    node->addChild( new ParseNode( match( TokenType::SEMICOLON ) ) );
    node->addChild( parseBlock() );
    node->addChild( new ParseNode( match( TokenType::SEMICOLON ) ) );

    return node;
}

// formal-parameter-list -> lparent + parameter-group + (semicolon + parameter-group)* + rparent
ParseNode* Parser::parseFormalParameterList() {
    ParseNode* node = new ParseNode( "<formal-parameter-list>" );

    node->addChild( new ParseNode( match( TokenType::LPARENT ) ) );
    node->addChild( parseParameterGroup() );

    while (!isAtEnd() && currentToken().type == TokenType::SEMICOLON) {
        node->addChild( new ParseNode( match( TokenType::SEMICOLON ) ) );
        node->addChild( parseParameterGroup() );
    }

    node->addChild( new ParseNode( match( TokenType::RPARENT ) ) );
    return node;
}

// parameter-group -> identifier-list + colon + (ident | array-type)
ParseNode* Parser::parseParameterGroup() {
    ParseNode* node = new ParseNode( "<parameter-group>" );

    node->addChild( parseIdentifierList() );
    node->addChild( new ParseNode( match( TokenType::COLON ) ) );

    if (currentToken().type == TokenType::ARRAY) {
        node->addChild( parseArrayType() );
    }
    else {
        node->addChild( new ParseNode( match( TokenType::IDENTIFIER ) ) );
    }

    return node;
}
