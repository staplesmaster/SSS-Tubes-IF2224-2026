#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "ASTVisitor.hpp"
#include "ASTNode.hpp"
#include "SymbolTable.hpp"
#include "ErrorReporter.hpp"

class SemanticAnalyzer : public ASTVisitor {
    private: 
        SymbolTable symbolTable;
        ErrorReporter errorReporter;
        std::vector<int> activeBlocks;
        std::unordered_map<ASTNode*, int> enumDomainIds;
        int nextEnumDomainId = 1;

        ExprType resolveTypeNode(ASTNode* typeNode);
        int getEnumDomainIdForTypeNode(ASTNode* typeNode);
        int getEnumDomainIdForExpression(ASTNode* expr);
        bool areTypeNodesCompatible(ASTNode* leftTypeNode, ASTNode* rightTypeNode);
        bool tryGetIntLiteral(ASTNode* node, int& value);
        ExprType resolveRecordFieldType(ASTNode* typeNode, const std::string& fieldName, int& outOffset);
        ASTNode* getTypeDefForExpression(ASTNode* expr);
        void report(ASTNode* node, const std::string& message);

    public:
        SemanticAnalyzer() = default;

        void analyze(ASTNode* root);

        const ErrorReporter& getErrorReporter() const;
        const SymbolTable& getSymbolTable() const;

        SymbolTable& getSymbolTable();

        int getTypeWidth(ASTNode* typeNode);

        void visitProgramNode(ProgramNode* node) override;
        void visitCompoundNode(CompoundNode* node) override;
        void visitConstDeclNode(ConstDeclNode* node) override;
        void visitTypeDeclNode(TypeDeclNode* node) override;
        void visitVarDeclNode(VarDeclNode* node) override;
        void visitParamNode(ParamNode* node) override;
        void visitSubprogramDeclNode(SubprogramDeclNode* node) override;

        void visitNamedTypeNode(NamedTypeNode* node) override;
        void visitArrayTypeNode(ArrayTypeNode* node) override;
        void visitRecordTypeNode(RecordTypeNode* node) override;
        void visitRangeNode(RangeNode* node) override;
        void visitEnumNode(EnumNode* node) override;

        void visitAssignNode(AssignNode* node) override;
        void visitIfNode(IfNode* node) override;
        void visitCaseBlockNode(CaseBlockNode* node) override;
        void visitCaseNode(CaseNode* node) override;
        void visitWhileNode(WhileNode* node) override;
        void visitRepeatNode(RepeatNode* node) override;
        void visitForNode(ForNode* node) override;
        void visitProcCallNode(ProcCallNode* node) override;

        void visitVarNode(VarNode* node) override;
        void visitArrayAccessNode(ArrayAccessNode* node) override;
        void visitRecordAccessNode(RecordAccessNode* node) override;

        void visitBinOpNode(BinOpNode* node) override;
        void visitUnaryOpNode(UnaryOpNode* node) override;
        void visitNumberNode(NumberNode* node) override;
        void visitStringNode(StringNode* node) override;
        void visitCharNode(CharNode* node) override;

};