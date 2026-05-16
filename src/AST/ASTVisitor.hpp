#ifndef ASTVISITOR_HPP
#define ASTVISITOR_HPP

// Forward declarations untuk semua tipe Node
class ProgramNode; class CompoundNode; class ConstDeclNode; class TypeDeclNode;
class VarDeclNode; class ParamNode; class SubprogramDeclNode; class NamedTypeNode;
class ArrayTypeNode; class RecordTypeNode; class RangeNode; class EnumNode;
class AssignNode; class IfNode; class CaseBlockNode; class CaseNode;
class WhileNode; class RepeatNode; class ForNode; class ProcCallNode;
class VarNode; class ArrayAccessNode; class RecordAccessNode; class BinOpNode;
class UnaryOpNode; class NumberNode; class StringNode; class CharNode;

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visitProgramNode(ProgramNode* node) = 0;
    virtual void visitCompoundNode(CompoundNode* node) = 0;
    virtual void visitConstDeclNode(ConstDeclNode* node) = 0;
    virtual void visitTypeDeclNode(TypeDeclNode* node) = 0;
    virtual void visitVarDeclNode(VarDeclNode* node) = 0;
    virtual void visitParamNode(ParamNode* node) = 0;
    virtual void visitSubprogramDeclNode(SubprogramDeclNode* node) = 0;
    
    virtual void visitNamedTypeNode(NamedTypeNode* node) = 0;
    virtual void visitArrayTypeNode(ArrayTypeNode* node) = 0;
    virtual void visitRecordTypeNode(RecordTypeNode* node) = 0;
    virtual void visitRangeNode(RangeNode* node) = 0;
    virtual void visitEnumNode(EnumNode* node) = 0;

    virtual void visitAssignNode(AssignNode* node) = 0;
    virtual void visitIfNode(IfNode* node) = 0;
    virtual void visitCaseBlockNode(CaseBlockNode* node) = 0;
    virtual void visitCaseNode(CaseNode* node) = 0;
    virtual void visitWhileNode(WhileNode* node) = 0;
    virtual void visitRepeatNode(RepeatNode* node) = 0;
    virtual void visitForNode(ForNode* node) = 0;
    virtual void visitProcCallNode(ProcCallNode* node) = 0;

    virtual void visitVarNode(VarNode* node) = 0;
    virtual void visitArrayAccessNode(ArrayAccessNode* node) = 0;
    virtual void visitRecordAccessNode(RecordAccessNode* node) = 0;

    virtual void visitBinOpNode(BinOpNode* node) = 0;
    virtual void visitUnaryOpNode(UnaryOpNode* node) = 0;
    virtual void visitNumberNode(NumberNode* node) = 0;
    virtual void visitStringNode(StringNode* node) = 0;
    virtual void visitCharNode(CharNode* node) = 0;
};

#endif