#ifndef ASTNODE_HPP
#define ASTNODE_HPP

#include <string>
#include <vector>

using namespace std;

class ASTVisitor;

enum class ExprType {
    UNKNOWN, INTEGER, REAL, BOOLEAN, CHAR, STRING, VOID, ARRAY, RECORD, ENUM, SUBRANGE
};

// Base Class
class ASTNode {
public:
    ExprType exprType = ExprType::UNKNOWN; 
    int tabIndex = -1; // Referensi indeks di symbol table (tab)
    int lev = -1;      // Kedalaman scope (0 = global)
    int lineNum = 0;   // Baris node ini berasal dari source code

    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor* visitor) = 0; 
};

// STRUKTUR PROGRAM
class ProgramNode : public ASTNode {
public:
    string programName;
    vector<ASTNode*> declarations; 
    ASTNode* mainBlock;            

    ProgramNode(string name, vector<ASTNode*> decls, ASTNode* block);
    void accept(ASTVisitor* visitor) override;
};

class CompoundNode : public ASTNode {
public:
    vector<ASTNode*> statements; 

    CompoundNode(vector<ASTNode*> stmts);
    void accept(ASTVisitor* visitor) override;
};

// DEKLARASI
class ConstDeclNode : public ASTNode {
public:
    string constName;
    ASTNode* value;

    ConstDeclNode(string name, ASTNode* val);
    void accept(ASTVisitor* visitor) override;
};

class TypeDeclNode : public ASTNode {
public:
    string typeName;
    ASTNode* typeDef;

    TypeDeclNode(string name, ASTNode* def);
    void accept(ASTVisitor* visitor) override;
};

class VarDeclNode : public ASTNode {
public:
    vector<string> varNames;
    ASTNode* typeDef;

    VarDeclNode(vector<string> names, ASTNode* def);
    void accept(ASTVisitor* visitor) override;
};

class ParamNode : public ASTNode {
public:
    vector<string> paramNames;
    ASTNode* typeDef;
    bool isVarParam; // True jika pass-by-reference (var parameter)

    ParamNode(vector<string> names, ASTNode* def, bool isVar);
    void accept(ASTVisitor* visitor) override;
};

class SubprogramDeclNode : public ASTNode {
public:
    bool isFunction;
    string subprogramName;
    vector<ASTNode*> parameters;
    ASTNode* returnType; // nullptr jika Procedure
    vector<ASTNode*> declarations;
    ASTNode* body;

    SubprogramDeclNode(bool isFunc, string name, vector<ASTNode*> params, 
                       ASTNode* retType, vector<ASTNode*> decls, ASTNode* bodyBlock);
    void accept(ASTVisitor* visitor) override;
};

// DEFINISI TIPE
class NamedTypeNode : public ASTNode {
public:
    string typeName; // Misal: "integer", "Hari"

    NamedTypeNode(string name);
    void accept(ASTVisitor* visitor) override;
};

class ArrayTypeNode : public ASTNode {
public:
    ASTNode* indexType;
    ASTNode* elementType;

    ArrayTypeNode(ASTNode* idxType, ASTNode* elemType);
    void accept(ASTVisitor* visitor) override;
};

class RecordTypeNode : public ASTNode {
public:
    vector<ASTNode*> fields; // Isinya VarDeclNode

    RecordTypeNode(vector<ASTNode*> recFields);
    void accept(ASTVisitor* visitor) override;
};

class RangeNode : public ASTNode {
public:
    ASTNode* lowerBound;
    ASTNode* upperBound;

    RangeNode(ASTNode* low, ASTNode* high);
    void accept(ASTVisitor* visitor) override;
};

class EnumNode : public ASTNode {
public:
    vector<string> identifiers;

    EnumNode(vector<string> ids);
    void accept(ASTVisitor* visitor) override;
};

// STATEMENT
class AssignNode : public ASTNode {
public:
    ASTNode* target;
    ASTNode* value;

    AssignNode(ASTNode* tgt, ASTNode* val);
    void accept(ASTVisitor* visitor) override;
};

class IfNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* thenBlock;
    ASTNode* elseBlock;

    IfNode(ASTNode* cond, ASTNode* tBlock, ASTNode* eBlock);
    void accept(ASTVisitor* visitor) override;
};

class CaseBlockNode : public ASTNode {
public:
    vector<ASTNode*> constants;
    ASTNode* statement;

    CaseBlockNode(vector<ASTNode*> consts, ASTNode* stmt);
    void accept(ASTVisitor* visitor) override;
};

class CaseNode : public ASTNode {
public:
    ASTNode* condition;
    vector<ASTNode*> cases; // Isinya CaseBlockNode

    CaseNode(ASTNode* cond, vector<ASTNode*> caseList);
    void accept(ASTVisitor* visitor) override;
};

class WhileNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* loopBlock;

    WhileNode(ASTNode* cond, ASTNode* block);
    void accept(ASTVisitor* visitor) override;
};

class RepeatNode : public ASTNode {
public:
    vector<ASTNode*> statements;
    ASTNode* condition;

    RepeatNode(vector<ASTNode*> stmts, ASTNode* cond);
    void accept(ASTVisitor* visitor) override;
};

class ForNode : public ASTNode {
public:
    string iteratorName;
    ASTNode* startValue;
    ASTNode* endValue;
    bool isDownto;
    ASTNode* loopBlock;

    ForNode(string iterName, ASTNode* startVal, ASTNode* endVal, bool downto, ASTNode* block);
    void accept(ASTVisitor* visitor) override;
};

class ProcCallNode : public ASTNode {
public:
    string procName;
    vector<ASTNode*> arguments;

    ProcCallNode(string name, vector<ASTNode*> args);
    void accept(ASTVisitor* visitor) override;
};

// COMPONENT VARIABLE
class VarNode : public ASTNode {
public:
    string name;

    VarNode(string n);
    void accept(ASTVisitor* visitor) override;
};

class ArrayAccessNode : public ASTNode {
public:
    ASTNode* arrayVar;
    vector<ASTNode*> indices;

    ArrayAccessNode(ASTNode* arrVar, vector<ASTNode*> idxs);
    void accept(ASTVisitor* visitor) override;
};

class RecordAccessNode : public ASTNode {
public:
    ASTNode* recordVar;
    string fieldName;

    RecordAccessNode(ASTNode* recVar, string fName);
    void accept(ASTVisitor* visitor) override;
};

// EXPRESSION & FACTOR
class BinOpNode : public ASTNode {
public:
    string op; 
    ASTNode* left;
    ASTNode* right;

    BinOpNode(string oper, ASTNode* l, ASTNode* r);
    void accept(ASTVisitor* visitor) override;
};

class UnaryOpNode : public ASTNode {
public:
    string op; // "+" "-" "not"
    ASTNode* operand;

    UnaryOpNode(string oper, ASTNode* expr);
    void accept(ASTVisitor* visitor) override;
};

class NumberNode : public ASTNode {
public:
    string value; 
    bool isReal;  

    NumberNode(string val, bool isR);
    void accept(ASTVisitor* visitor) override;
};

class StringNode : public ASTNode {
public:
    string value;

    StringNode(string val);
    void accept(ASTVisitor* visitor) override;
};

class CharNode : public ASTNode {
public:
    string value;

    CharNode(string val);
    void accept(ASTVisitor* visitor) override;
};

#endif