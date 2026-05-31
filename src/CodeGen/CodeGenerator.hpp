#ifndef CODEGENERATOR_HPP
#define CODEGENERATOR_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include "ASTVisitor.hpp"
#include "ASTNode.hpp"
#include "SymbolTable.hpp"
#include "Instruction.hpp"

class CodeGenerator : public ASTVisitor {
private:
    SymbolTable* symbolTable;
    std::vector<Instruction> instructions;
    int currentLevel = 0;
    bool isLValueMode = false;
    void emit(const Instruction& instr);
    int resolveTabIndex(ASTNode* node) const;
    int resolveLevel(ASTNode* node) const;
    int levelDiff(ASTNode* node) const;

    OprCode mapBinaryOpr(const std::string& op) const;
    OprCode mapUnaryOpr(const std::string& op) const;
    bool isRelationalOp(const std::string& op) const;

public:
    explicit CodeGenerator(SymbolTable& table);

    void generate(ASTNode* root);
    const std::vector<Instruction>& getInstructions() const;
    std::string dump() const;

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

#endif
