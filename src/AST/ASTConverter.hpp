#ifndef ASTCONVERTER_HPP
#define ASTCONVERTER_HPP

#include "ASTNode.hpp"
#include "ParseNode.hpp"
#include <vector>
#include <string>

class ASTConverter {
public:
    ASTNode* build(ParseNode* parseRoot);

private:
    // STRUKTUR PROGRAM
    ASTNode* convertProgram(ParseNode* node);
    ASTNode* convertCompoundStatement(ParseNode* node);
    vector<ASTNode*> convertStatementList(ParseNode* node);

    // DEKLARASI
    vector<ASTNode*> convertDeclarationPart(ParseNode* node);
    vector<ASTNode*> convertConstDeclaration(ParseNode* node);
    vector<ASTNode*> convertTypeDeclaration(ParseNode* node);
    vector<ASTNode*> convertVarDeclaration(ParseNode* node);
    vector<string> convertIdentifierList(ParseNode* node);
    ASTNode* convertSubprogramDeclaration(ParseNode* node);
    vector<ASTNode*> convertFormalParameterList(ParseNode* node);

    // TIPE DATA
    ASTNode* convertType(ParseNode* node);
    ASTNode* convertConstant(ParseNode* node);
    ASTNode* convertArrayType(ParseNode* node);
    ASTNode* convertRange(ParseNode* node);
    ASTNode* convertEnumerated(ParseNode* node);
    ASTNode* convertRecordType(ParseNode* node);

    // STATEMENT & COMPONENT VARIABLE
    ASTNode* convertStatement(ParseNode* node);
    ASTNode* convertAssignment(ParseNode* node);
    ASTNode* convertIf(ParseNode* node);
    ASTNode* convertCase(ParseNode* node);
    ASTNode* convertCaseBlock(ParseNode* node);
    ASTNode* convertWhile(ParseNode* node);
    ASTNode* convertRepeat(ParseNode* node);
    ASTNode* convertFor(ParseNode* node);
    ASTNode* convertProcCall(ParseNode* node);
    // helper
    ASTNode* convertVariable(ParseNode* node);
    vector<ASTNode*> convertIndexList(ParseNode* node);
    vector<ASTNode*> convertParameterList(ParseNode* node);

    // EXPRESSION & FACTOR
    ASTNode* convertExpression(ParseNode* node);
    ASTNode* convertSimpleExpression(ParseNode* node);
    ASTNode* convertTerm(ParseNode* node);
    ASTNode* convertFactor(ParseNode* node);
};

#endif