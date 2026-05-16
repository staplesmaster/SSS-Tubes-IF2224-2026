#include "ASTNode.hpp"
#include "ASTVisitor.hpp"

// STRUKTUR PROGRAM
ProgramNode::ProgramNode(string name, vector<ASTNode*> decls, ASTNode* block)
    : programName(name), declarations(decls), mainBlock(block) {}
void ProgramNode::accept(ASTVisitor* visitor) { visitor->visitProgramNode(this); }

const string& ProgramNode::getProgramName() {
    return programName;
}

CompoundNode::CompoundNode(vector<ASTNode*> stmts) : statements(stmts) {}
void CompoundNode::accept(ASTVisitor* visitor) { visitor->visitCompoundNode(this); }

// DEKLARASI
ConstDeclNode::ConstDeclNode(string name, ASTNode* val) : constName(name), value(val) {}
void ConstDeclNode::accept(ASTVisitor* visitor) { visitor->visitConstDeclNode(this); }

string ConstDeclNode::getConstName() {
    return constName;
}

ASTNode* ConstDeclNode::getValue() {
    return value;
}


TypeDeclNode::TypeDeclNode(string name, ASTNode* def) : typeName(name), typeDef(def) {}
void TypeDeclNode::accept(ASTVisitor* visitor) { visitor->visitTypeDeclNode(this); }

string TypeDeclNode::getTypeName() {
    return typeName;
}

VarDeclNode::VarDeclNode(vector<string> names, ASTNode* def) : varNames(names), typeDef(def) {}
void VarDeclNode::accept(ASTVisitor* visitor) { visitor->visitVarDeclNode(this); }

vector<string> VarDeclNode::getVarNames() {
    return varNames;
}

ASTNode* VarDeclNode::getTypeDef() {
    return typeDef;
}

ParamNode::ParamNode(vector<string> names, ASTNode* def, bool isVar)
    : paramNames(names), typeDef(def), isVarParam(isVar) {}
void ParamNode::accept(ASTVisitor* visitor) { visitor->visitParamNode(this); }

SubprogramDeclNode::SubprogramDeclNode(bool isFunc, string name, vector<ASTNode*> params, 
                                       ASTNode* retType, vector<ASTNode*> decls, ASTNode* bodyBlock)
    : isFunction(isFunc), subprogramName(name), parameters(params), 
      returnType(retType), declarations(decls), body(bodyBlock) {}
void SubprogramDeclNode::accept(ASTVisitor* visitor) { visitor->visitSubprogramDeclNode(this); }

string SubprogramDeclNode::getSubprogramName() {
    return subprogramName;
}

ASTNode* SubprogramDeclNode::getBody() {
    return body;
}

// DEFINISI TIPE
NamedTypeNode::NamedTypeNode(string name) : typeName(name) {}
void NamedTypeNode::accept(ASTVisitor* visitor) { visitor->visitNamedTypeNode(this); }

ArrayTypeNode::ArrayTypeNode(ASTNode* idxType, ASTNode* elemType)
    : indexType(idxType), elementType(elemType) {}
void ArrayTypeNode::accept(ASTVisitor* visitor) { visitor->visitArrayTypeNode(this); }

RecordTypeNode::RecordTypeNode(vector<ASTNode*> recFields) : fields(recFields) {}
void RecordTypeNode::accept(ASTVisitor* visitor) { visitor->visitRecordTypeNode(this); }

RangeNode::RangeNode(ASTNode* low, ASTNode* high) : lowerBound(low), upperBound(high) {}
void RangeNode::accept(ASTVisitor* visitor) { visitor->visitRangeNode(this); }

EnumNode::EnumNode(vector<string> ids) : identifiers(ids) {}
void EnumNode::accept(ASTVisitor* visitor) { visitor->visitEnumNode(this); }

// STATEMENT
AssignNode::AssignNode(ASTNode* tgt, ASTNode* val) : target(tgt), value(val) {}
void AssignNode::accept(ASTVisitor* visitor) { visitor->visitAssignNode(this); }

IfNode::IfNode(ASTNode* cond, ASTNode* tBlock, ASTNode* eBlock)
    : condition(cond), thenBlock(tBlock), elseBlock(eBlock) {}
void IfNode::accept(ASTVisitor* visitor) { visitor->visitIfNode(this); }

CaseBlockNode::CaseBlockNode(vector<ASTNode*> consts, ASTNode* stmt)
    : constants(consts), statement(stmt) {}
void CaseBlockNode::accept(ASTVisitor* visitor) { visitor->visitCaseBlockNode(this); }

CaseNode::CaseNode(ASTNode* cond, vector<ASTNode*> caseList)
    : condition(cond), cases(caseList) {}
void CaseNode::accept(ASTVisitor* visitor) { visitor->visitCaseNode(this); }

WhileNode::WhileNode(ASTNode* cond, ASTNode* block) : condition(cond), loopBlock(block) {}
void WhileNode::accept(ASTVisitor* visitor) { visitor->visitWhileNode(this); }

RepeatNode::RepeatNode(vector<ASTNode*> stmts, ASTNode* cond)
    : statements(stmts), condition(cond) {}
void RepeatNode::accept(ASTVisitor* visitor) { visitor->visitRepeatNode(this); }

ForNode::ForNode(string iterName, ASTNode* startVal, ASTNode* endVal, bool downto, ASTNode* block)
    : iteratorName(iterName), startValue(startVal), endValue(endVal), isDownto(downto), loopBlock(block) {}
void ForNode::accept(ASTVisitor* visitor) { visitor->visitForNode(this); }

ProcCallNode::ProcCallNode(string name, vector<ASTNode*> args)
    : procName(name), arguments(args) {}
void ProcCallNode::accept(ASTVisitor* visitor) { visitor->visitProcCallNode(this); }

vector<ASTNode*> ProcCallNode::getArguments() {
    return arguments;
}

// COMPONENT VARIABLE
VarNode::VarNode(string n) : name(n) {}
void VarNode::accept(ASTVisitor* visitor) { visitor->visitVarNode(this); }

ArrayAccessNode::ArrayAccessNode(ASTNode* arrVar, vector<ASTNode*> idxs)
    : arrayVar(arrVar), indices(idxs) {}
void ArrayAccessNode::accept(ASTVisitor* visitor) { visitor->visitArrayAccessNode(this); }

ASTNode* ArrayAccessNode::getArrayVar() {
    return arrayVar;
}

RecordAccessNode::RecordAccessNode(ASTNode* recVar, string fName)
    : recordVar(recVar), fieldName(fName) {}
void RecordAccessNode::accept(ASTVisitor* visitor) { visitor->visitRecordAccessNode(this); }

ASTNode* RecordAccessNode::getRecordVar() {
    return recordVar;
}

// EXPRESSION & FACTOR
BinOpNode::BinOpNode(string oper, ASTNode* l, ASTNode* r) : op(oper), left(l), right(r) {}
void BinOpNode::accept(ASTVisitor* visitor) { visitor->visitBinOpNode(this); }

UnaryOpNode::UnaryOpNode(string oper, ASTNode* expr) : op(oper), operand(expr) {}
void UnaryOpNode::accept(ASTVisitor* visitor) { visitor->visitUnaryOpNode(this); }

NumberNode::NumberNode(string val, bool isR) : value(val), isReal(isR) {}
void NumberNode::accept(ASTVisitor* visitor) { visitor->visitNumberNode(this); }

StringNode::StringNode(string val) : value(val) {}
void StringNode::accept(ASTVisitor* visitor) { visitor->visitStringNode(this); }

CharNode::CharNode(string val) : value(val) {}
void CharNode::accept(ASTVisitor* visitor) { visitor->visitCharNode(this); }