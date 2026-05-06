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

// STRUKTUR PROGRAM UTAMA
// program -> program-eader + declaration-part + compound-statement + period
ParseNode* Parser::parseProgram(){
    ParseNode* node = new ParseNode("<program>");

    node->addChild(parseProgramHeader());
    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());
    node->addChild(new ParseNode(match(TokenType::PERIOD)));

    return node;
}

// program-header -> programsy + ident + semicolon
ParseNode* Parser::parseProgramHeader(){
    ParseNode* node = new ParseNode("<program-header>");

    node->addChild(new ParseNode(match(TokenType::PROGRAM)));
    node->addChild(new ParseNode(match(TokenType::IDENTIFIER)));
    node->addChild(new ParseNode(match(TokenType::SEMICOLON)));

    return node; 

}

// block -> declaration-part + compound-statement
ParseNode* Parser::parseBlock(){
    ParseNode* node = new ParseNode("<block>");

    node->addChild(parseDeclarationPart());
    node->addChild(parseCompoundStatement());

    return node;
}


// DEKLARASI
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

// TIPE DATA
// type -> ident | array-type | range | enumerated | record-type
ParseNode* Parser::parseType(){
    ParseNode* node = new ParseNode("<type>");
    
    if (currentToken().type == TokenType::ARRAY){
        node->addChild(parseArrayType());
    } else if (currentToken().type == TokenType::LPARENT){
        node->addChild(parseEnumerated());
    } else if (currentToken().type == TokenType::RECORD){
        node->addChild(parseRecordType());
    } else if (currentToken().type == TokenType::IDENTIFIER){
        if (peek(1).type == TokenType::PERIOD){ // Kalau token selanjutnya . berarti masuk ke range
            node->addChild(parseRange());
        } else {
            node->addChild(new ParseNode(match(TokenType::IDENTIFIER)));
        }
    } else if (currentToken().type == TokenType::INTCON || currentToken().type == TokenType::REALCON || currentToken().type == TokenType::CHARCON ||
                currentToken().type == TokenType::STRING || currentToken().type == TokenType::PLUS || currentToken().type == TokenType::MINUS){
            node->addChild(parseRange()); // ini untuk string harus cek lagi sih
    } else {
        node->addChild(new ParseNode(match(TokenType::UNKNOWN)));
    }
    return node;
}

// array-type -> arraysy + lbrack + (range | ident) + rbrack + ofsy + type
ParseNode* Parser::parseArrayType(){
    ParseNode* node = new ParseNode("<array-type>");

    node->addChild(new ParseNode(match(TokenType::ARRAY)));
    node->addChild(new ParseNode(match(TokenType::LBRACK)));
    
    if (currentToken().type == TokenType::IDENTIFIER && peek(1).type != TokenType::PERIOD){
        node->addChild(new ParseNode(match(TokenType::IDENTIFIER)));
    } else {
        node->addChild(parseRange()); 
    }

    node->addChild(new ParseNode(match(TokenType::RBRACK)));
    node->addChild(new ParseNode(match(TokenType::OF)));
    node->addChild(parseType());

    return node;
}

// range -> constant + period + period + constant
ParseNode* Parser::parseRange(){
    ParseNode* node = new ParseNode("<range>");

    node->addChild(parseConstant()); // batas bawah
    node->addChild(new ParseNode(match(TokenType::PERIOD)));
    node->addChild(new ParseNode(match(TokenType::PERIOD)));
    node->addChild(parseConstant()); // batas atas
    
    return node;
}

// enumerated -> lparent + ident + (comma + ident)* + rparent
ParseNode* Parser::parseEnumerated(){
    ParseNode* node = new ParseNode("<enumerated>");

    node->addChild(new ParseNode(match(TokenType::LPARENT)));
    node->addChild(new ParseNode(match(TokenType::IDENTIFIER)));
    
    while (!isAtEnd() && currentToken().type == TokenType::COMMA){
        node->addChild(new ParseNode(match(TokenType::COMMA)));
        node->addChild(new ParseNode(match(TokenType::IDENTIFIER)));
    }
    node->addChild(new ParseNode(match(TokenType::RPARENT)));
    return node;
}

// record-type -> recordsy + field-list + endsy
ParseNode* Parser::parseRecordType(){
    ParseNode* node = new ParseNode("<record-type>");

    node->addChild(new ParseNode(match(TokenType::RECORD)));
    node->addChild(parseFieldList());
    node->addChild(new ParseNode(match(TokenType::END)));

    return node;
}

// field-list -> field-part + (semicolon + field-part)*
ParseNode* Parser::parseFieldList(){
    ParseNode* node = new ParseNode("<field-list>");

    node->addChild(parseFieldPart());
    while (!isAtEnd() && currentToken().type == TokenType::SEMICOLON && peek(1).type == TokenType::IDENTIFIER){
        node->addChild(new ParseNode(match(TokenType::SEMICOLON)));
        node->addChild(parseFieldPart());
    }
    return node;
}

// field-part -> identifier-lsit + colon + type
ParseNode* Parser::parseFieldPart(){
    ParseNode* node = new ParseNode("<field-part>");

    node->addChild(parseIdentifierList());
    node->addChild(new ParseNode(match(TokenType::COLON)));
    node->addChild(parseType());
    return node;
}

// STATEMENT 
// compound-statement -> beginsy + statement-list + endsy
ParseNode* Parser::parseCompoundStatement(){
    ParseNode* node = new ParseNode("<compound-statement>");

    node->addChild(new ParseNode(match(TokenType::BEGIN)));
    node->addChild(parseStatementList());
    node->addChild(new ParseNode(match(TokenType::END)));

    return node;
}

// statement-list -> statement + (semicolon + statement)*
ParseNode* Parser::parseStatementList(){
    ParseNode* node = new ParseNode("<statement-list>");
    node->addChild(parseStatement());
    while (!isAtEnd() && currentToken().type == TokenType::SEMICOLON){
        node->addChild(new ParseNode(match(TokenType::SEMICOLON)));
        node->addChild(parseStatement());
    }
    return node;
}

// statement -> (assignment-state | if-statement | case-statement | while-statement | repeat-statement | for-statement | procedure/function-call)?
ParseNode* Parser::parseStatement(){
    ParseNode* node = new ParseNode("<statement>");
    TokenType t = currentToken().type;

    if (t == TokenType::IF){
        node->addChild(parseIfStatement());
    } else if (t == TokenType::CASE){
        node->addChild(parseCaseStatement());
    } else if (t == TokenType::WHILE){
        node->addChild(parseWhileStatement());
    } else if (t == TokenType::REPEAT){
        node->addChild(parseRepeatStatement());
    } else if (t == TokenType::FOR){
        node->addChild(parseForStatement());
    } else if(t == TokenType::IDENTIFIER){
        TokenType nextT = peek(1).type;

        // buat nanganin a := .., a[1] := .., a.field := ..
        if (nextT == TokenType::ASSIGN || nextT ==TokenType::LBRACK || nextT == TokenType::PERIOD){
            node->addChild(parseAssignmentStatement());
        } else if(nextT == TokenType::LPARENT ||nextT == TokenType::SEMICOLON || nextT == TokenType::END
                 || nextT == TokenType::ELSE || nextT == TokenType::UNTIL){
                    node->addChild(parseProcedureFunctionCall());
            } else { 
        node->addChild(new ParseNode(match(TokenType::UNKNOWN)));
        }
    } 
    return node; 
}

// variable -> ident + (component-variable)*
ParseNode* Parser::parseVariable() {
    ParseNode* node = new ParseNode("<variable>");
    
    node->addChild(new ParseNode(match(TokenType::IDENTIFIER)));
    
    // Looping selama awalan variabel ('[' atau '.')
    while (!isAtEnd() && (currentToken().type == TokenType::LBRACK || currentToken().type == TokenType::PERIOD)) {
        node->addChild(parseComponentVariable());
    }
    
    return node;
}

// component-variable -> (lbrack + index-list + rbrack) | (period + ident)
ParseNode* Parser::parseComponentVariable() {
    ParseNode* node = new ParseNode("<component-variable>");
    
    if (currentToken().type == TokenType::LBRACK) {
        node->addChild(new ParseNode(match(TokenType::LBRACK)));
        node->addChild(parseIndexList());
        node->addChild(new ParseNode(match(TokenType::RBRACK)));
    } else if (currentToken().type == TokenType::PERIOD) {
        node->addChild(new ParseNode(match(TokenType::PERIOD)));
        node->addChild(new ParseNode(match(TokenType::IDENTIFIER)));
    }
    
    return node;
}

// index-list -> (intcon | charcon | ident) + (comma + index-list)*
ParseNode* Parser::parseIndexList() {
    ParseNode* node = new ParseNode("<index-list>");
    TokenType t = currentToken().type;
    
    if (t == TokenType::INTCON || t == TokenType::CHARCON || t == TokenType::IDENTIFIER) {
        node->addChild(new ParseNode(match(t)));
    } else {
        node->addChild(new ParseNode(match(TokenType::UNKNOWN)));
    }
    
    if (currentToken().type == TokenType::COMMA) {
        node->addChild(new ParseNode(match(TokenType::COMMA)));
        node->addChild(parseIndexList()); // Rekursif (comma + index-list)*
    }
    
    return node;
}

// assignment-statement -> variable + becomes + expression
ParseNode* Parser::parseAssignmentStatement() {
    ParseNode* node = new ParseNode("<assignment-statement>");
    
    node->addChild(parseVariable());
    node->addChild(new ParseNode(match(TokenType::ASSIGN))); 
    node->addChild(parseExpression());
    
    return node;
}

// if-statement -> ifsy + expression + thensy + statement + (elsy + statement)?
ParseNode* Parser::parseIfStatement() {
    ParseNode* node = new ParseNode("<if-statement>");
    
    node->addChild(new ParseNode(match(TokenType::IF)));
    node->addChild(parseExpression());
    node->addChild(new ParseNode(match(TokenType::THEN)));
    node->addChild(parseStatement());
    
    if (currentToken().type == TokenType::ELSE) {
        node->addChild(new ParseNode(match(TokenType::ELSE)));
        node->addChild(parseStatement());
    }
    
    return node;
}

// case-statement -> casesy + expression + ofsy + case-block + endsy
ParseNode* Parser::parseCaseStatement() {
    ParseNode* node = new ParseNode("<case-statement>");
    
    node->addChild(new ParseNode(match(TokenType::CASE)));
    node->addChild(parseExpression());
    node->addChild(new ParseNode(match(TokenType::OF)));
    node->addChild(parseCaseBlock());
    node->addChild(new ParseNode(match(TokenType::END)));
    
    return node;
}

// case-block -> constant + (comma + constant)* + colon + statement + (semicolon + case-block?)*
ParseNode* Parser::parseCaseBlock() {
    ParseNode* node = new ParseNode("<case-block>");
    
    node->addChild(parseConstant());
    
    while (!isAtEnd() && currentToken().type == TokenType::COMMA) {
        node->addChild(new ParseNode(match(TokenType::COMMA)));
        node->addChild(parseConstant());
    }
    
    node->addChild(new ParseNode(match(TokenType::COLON)));
    node->addChild(parseStatement());
    
    // Looping case-block lanjutan yang dipisahkan semicolon
    while (!isAtEnd() && currentToken().type == TokenType::SEMICOLON) {
        node->addChild(new ParseNode(match(TokenType::SEMICOLON)));
        
        // Kalau token selanjutnya 'end', jangan panggil caseBlock 
        if (currentToken().type != TokenType::END) {
            node->addChild(parseCaseBlock()); 
        }
    }
    
    return node;
}

ParseNode* Parser::parseWhileStatement() {
    ParseNode* node = new ParseNode("<while-statement>");
    
    node->addChild(new ParseNode(match(TokenType::WHILE)));
    node->addChild(parseExpression());
    node->addChild(new ParseNode(match(TokenType::DO)));
    node->addChild(parseStatement());
    
    return node;
}

// repeat-statement -> repeatsy + statement-list + untilsy + expression
ParseNode* Parser::parseRepeatStatement() {
    ParseNode* node = new ParseNode("<repeat-statement>");
    
    node->addChild(new ParseNode(match(TokenType::REPEAT)));
    node->addChild(parseStatementList());
    node->addChild(new ParseNode(match(TokenType::UNTIL)));
    node->addChild(parseExpression());
    
    return node;
}

// for-statement -> forsy + ident + becomes + expression + (tosy | downtosy) + expression + dosy + statement
ParseNode* Parser::parseForStatement() {
    ParseNode* node = new ParseNode("<for-statement>");
    
    node->addChild(new ParseNode(match(TokenType::FOR)));
    node->addChild(new ParseNode(match(TokenType::IDENTIFIER)));
    node->addChild(new ParseNode(match(TokenType::ASSIGN))); // becomes (:=)
    node->addChild(parseExpression());
    
    // Percabangan tosy ATAU downtosy
    if (currentToken().type == TokenType::TO || currentToken().type == TokenType::DOWNTO) {
        node->addChild(new ParseNode(match(currentToken().type)));
    } else {
        node->addChild(new ParseNode(match(TokenType::UNKNOWN))); // Menembak error jika tidak ada to/downto
    }
    
    node->addChild(parseExpression());
    node->addChild(new ParseNode(match(TokenType::DO)));
    node->addChild(parseStatement());
    
    return node;
}

// procedure/function-call -> ident + (lparent + parameter-list? + rparent)?
ParseNode* Parser::parseProcedureFunctionCall() {
    ParseNode* node = new ParseNode("<procedure/function-call>");
    
    node->addChild(new ParseNode(match(TokenType::IDENTIFIER)));
    
    // Tanda '?': Pemanggilan parameter opsional
    if (currentToken().type == TokenType::LPARENT) {
        node->addChild(new ParseNode(match(TokenType::LPARENT)));
        
        // Cek jika tidak langsung diakhiri kurung tutup, maka parsing parameter list
        if (currentToken().type != TokenType::RPARENT) {
            node->addChild(parseParameterList());
        }
        
        node->addChild(new ParseNode(match(TokenType::RPARENT)));
    }
    
    return node;
}

// parameter-list -> expression + (comma + expression)*
ParseNode* Parser::parseParameterList() {
    ParseNode* node = new ParseNode("<parameter-list>");
    
    node->addChild(parseExpression());
    
    while (!isAtEnd() && currentToken().type == TokenType::COMMA) {
        node->addChild(new ParseNode(match(TokenType::COMMA)));
        node->addChild(parseExpression());
    }
    
    return node;
}

// expression -> simple-expression + (relational-operator + simple-expression)?
ParseNode* Parser::parseExpression() {
    ParseNode* node = new ParseNode("<expression>");
    
    // Semua ekspresi pasti diawali dengan simple-expression
    node->addChild(parseSimpleExpression());
    
    // Tanda '?': Cek apakah ada operator perbandingan (==, !=, >, >=, <, <=)
    TokenType t = currentToken().type;
    if (t == TokenType::EQL || t == TokenType::NEQ || t == TokenType::GTR || 
        t == TokenType::GEQ || t == TokenType::LSS || t == TokenType::LEQ) {
        
        node->addChild(parseRelationalOperator());
        node->addChild(parseSimpleExpression());
    }
    
    return node;
}

// simple-expression -> (plus | minus)? + term + (additive-operator + term)*
ParseNode* Parser::parseSimpleExpression() {
    ParseNode* node = new ParseNode("<simple-expression>");
    TokenType t = currentToken().type;
    
    // Tanda '?': Prefix unary opsional (misal: -5 atau +10)
    if (t == TokenType::PLUS || t == TokenType::MINUS) {
        node->addChild(new ParseNode(match(t)));
    }
    
    // Masuk ke level prioritas berikutnya (term)
    node->addChild(parseTerm());
    
    // Looping '*': Penjumlahan atau pengurangan berantai (misal: a + b - c)
    t = currentToken().type;
    while (!isAtEnd() && (t == TokenType::PLUS || t == TokenType::MINUS || t == TokenType::OR)) {
        node->addChild(parseAdditiveOperator());
        node->addChild(parseTerm());
        t = currentToken().type; // Update intipan
    }
    
    return node;
}

// term -> factor + (multiplicative-operator + factor)*
ParseNode* Parser::parseTerm() {
    ParseNode* node = new ParseNode("<term>");
    
    // Masuk ke level prioritas tertinggi (factor)
    node->addChild(parseFactor());
    
    // Looping '*': Perkalian atau pembagian berantai (misal: x * y / z)
    TokenType t = currentToken().type;
    while (!isAtEnd() && (t == TokenType::TIMES || t == TokenType::RDIV || 
                          t == TokenType::IDIV || t == TokenType::MOD || t == TokenType::AND)) {
        node->addChild(parseMultiplicativeOperator());
        node->addChild(parseFactor());
        t = currentToken().type; // Update intipan
    }
    
    return node;
}

// factor -> ident | intcon | realcon | charcon | string | (lparent + expression + rparent) | (notsy + factor) | procedure/function-call | variable
ParseNode* Parser::parseFactor() {
    ParseNode* node = new ParseNode("<factor>");
    TokenType t = currentToken().type;
    
    // Rute 1: Tipe data konstan
    if (t == TokenType::INTCON || t == TokenType::REALCON || 
        t == TokenType::CHARCON || t == TokenType::STRING) {
        node->addChild(new ParseNode(match(t)));
    } 
    // Rute 2: Ekspresi di dalam kurung (misal: (a + b) )
    else if (t == TokenType::LPARENT) {
        node->addChild(new ParseNode(match(TokenType::LPARENT)));
        node->addChild(parseExpression());
        node->addChild(new ParseNode(match(TokenType::RPARENT)));
    } 
    // Rute 3: Unary NOT (misal: not a)
    else if (t == TokenType::NOT) {
        node->addChild(new ParseNode(match(TokenType::NOT)));
        node->addChild(parseFactor());
    } 
    // Rute 4: PENYELESAIAN AMBIGUITAS (ident vs func-call vs variable)
    else if (t == TokenType::IDENTIFIER) {
        // Jika identifier langsung diikuti kurung buka, PASTI pemanggilan fungsi
        if (peek(1).type == TokenType::LPARENT) {
            node->addChild(parseProcedureFunctionCall());
        } else {
            // Jika tidak, kita anggap sebagai variabel (baik itu skalar 'a', array 'a[1]', atau field 'a.x').
            // Catatan: Jika ini hanya identifier polos (skalar), fungsi parseVariable() 
            // akan memakannya dengan sempurna tanpa memanggil parseComponentVariable().
            node->addChild(parseVariable());
        }
    } 
    // Fallback: Syntax Error di tengah ekspresi
    else {
        node->addChild(new ParseNode(match(TokenType::UNKNOWN))); 
    }
    
    return node;
}

// relational-operator -> eql | neq | gtr | geq | lss | leq
ParseNode* Parser::parseRelationalOperator() {
    ParseNode* node = new ParseNode("<relational-operator>");
    TokenType t = currentToken().type;
    
    // Cek dengan pasti agar aman
    if (t == TokenType::EQL || t == TokenType::NEQ || t == TokenType::GTR || 
        t == TokenType::GEQ || t == TokenType::LSS || t == TokenType::LEQ) {
        node->addChild(new ParseNode(match(t)));
    } else {
        node->addChild(new ParseNode(match(TokenType::UNKNOWN)));
    }
    
    return node;
}

// additive-operator -> plus | minus | orsy
ParseNode* Parser::parseAdditiveOperator() {
    ParseNode* node = new ParseNode("<additive-operator>");
    TokenType t = currentToken().type;
    
    if (t == TokenType::PLUS || t == TokenType::MINUS || t == TokenType::OR) {
        node->addChild(new ParseNode(match(t)));
    } else {
        node->addChild(new ParseNode(match(TokenType::UNKNOWN)));
    }
    
    return node;
}

// multiplicative-operator -> times | rdiv | idiv | imod | andsy
ParseNode* Parser::parseMultiplicativeOperator() {
    ParseNode* node = new ParseNode("<multiplicative-operator>");
    TokenType t = currentToken().type;
    
    if (t == TokenType::TIMES || t == TokenType::RDIV || t == TokenType::IDIV || 
        t == TokenType::MOD || t == TokenType::AND) {
        node->addChild(new ParseNode(match(t)));
    } else {
        node->addChild(new ParseNode(match(TokenType::UNKNOWN)));
    }
    
    return node;
}