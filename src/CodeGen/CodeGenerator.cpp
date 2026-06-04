#include "CodeGenerator.hpp"
#include <stdexcept>

namespace {
std::string stripQuotedLiteral(const std::string& value) {
    if (value.size() >= 2) {
        const char first = value.front();
        const char last = value.back();
        if ((first == '\'' && last == '\'') || (first == '"' && last == '"')) {
            return value.substr(1, value.size() - 2);
        }
    }
    return value;
}
}

CodeGenerator::CodeGenerator(SymbolTable& table)
    : symbolTable(&table) {}

void CodeGenerator::emit(const Instruction& instr) {
    instructions.push_back(instr);
}

int CodeGenerator::resolveTabIndex(ASTNode* node) const {
    if (!node) return -1;
    return node->tabIndex;
}

int CodeGenerator::resolveLevel(ASTNode* node) const {
    if (!node) return -1;
    return node->lev;
}

int CodeGenerator::levelDiff(ASTNode* node) const {
    int declLevel = resolveLevel(node);
    if (declLevel < 0) return 0;
    return currentLevel - declLevel;
}

OprCode CodeGenerator::mapBinaryOpr(const std::string& op) const {
    if (op == "+") return OprCode::ADD;
    if (op == "-") return OprCode::SUB;
    if (op == "*") return OprCode::MUL;
    if (op == "/") return OprCode::DIV;
    if (op == "div") return OprCode::DIV;
    if (op == "mod") return OprCode::MOD;
    if (op == "=" || op == "==") return OprCode::EQL;
    if (op == "<>") return OprCode::NEQ;
    if (op == "<") return OprCode::LSS;
    if (op == ">") return OprCode::GTR;
    if (op == "<=") return OprCode::LEQ;
    if (op == ">=") return OprCode::GEQ;
    return OprCode::ADD;
}

OprCode CodeGenerator::mapUnaryOpr(const std::string& op) const {
    if (op == "-") return OprCode::NEG;
    return OprCode::NEG;
}

bool CodeGenerator::isRelationalOp(const std::string& op) const {
    return op == "=" || op == "==" || op == "<>" || op == "<" || op == ">" || op == "<=" || op == ">=";
}

void CodeGenerator::generate(ASTNode* root) {
    if (root) root->accept(this);
}

const std::vector<Instruction>& CodeGenerator::getInstructions() const {
    return instructions;
}

std::string CodeGenerator::dump() const {
    std::ostringstream out;
    for (size_t i = 0; i < instructions.size(); ++i) {
        out << i << " " << instructions[i] << "\n";
    }
    return out.str();
}

void CodeGenerator::visitProgramNode(ProgramNode* node) {
    currentLevel = 0;
    int varCount = 0;
    for (auto* decl : node->declarations) {
        if (auto* v = dynamic_cast<VarDeclNode*>(decl)) {
            varCount += static_cast<int>(v->varNames.size());
        }
    }

    emit(Instruction(PCodeOp::INT, 0, 3 + varCount));

    for (auto* decl : node->declarations) if (decl) decl->accept(this);

    if (node->mainBlock) node->mainBlock->accept(this);
    emit(Instruction(PCodeOp::RET, 0, 0));
}

void CodeGenerator::visitCompoundNode(CompoundNode* node) {
    for (auto* stmt : node->statements) {
        if (stmt) stmt->accept(this);
    }
}

void CodeGenerator::visitConstDeclNode(ConstDeclNode* node) {
    (void)node;
}

void CodeGenerator::visitTypeDeclNode(TypeDeclNode* node) { (void)node; }
void CodeGenerator::visitVarDeclNode(VarDeclNode* node) { (void)node; }
void CodeGenerator::visitParamNode(ParamNode* node) { (void)node; }

void CodeGenerator::visitSubprogramDeclNode(SubprogramDeclNode* node) {
    int skipSubprogramJmpIndex = static_cast<int>(instructions.size());
    emit(Instruction(PCodeOp::JMP, 0, 0)); 

    SymbolInfo* info = symbolTable->lookup(node->getSubprogramName());
    if (info) {
        info->entryAddress = static_cast<int>(instructions.size());
    }

    int savedLevel = currentLevel;
    currentLevel++;

    int localVarCount = 0;
    for (auto* decl : node->declarations) {
        if (auto* v = dynamic_cast<VarDeclNode*>(decl)) localVarCount += static_cast<int>(v->varNames.size());
    }
    int paramCount = 0;
    for (auto* p : node->parameters) if (auto* pn = dynamic_cast<ParamNode*>(p)) paramCount += static_cast<int>(pn->paramNames.size());

    emit(Instruction(PCodeOp::INT, 0, 3 + localVarCount));

    for (auto* decl : node->declarations) if (decl) decl->accept(this);
    if (node->body) node->body->accept(this);
    emit(Instruction(PCodeOp::RET, 0, 0));

    currentLevel = savedLevel;

    instructions[skipSubprogramJmpIndex].value = static_cast<int>(instructions.size());
}

void CodeGenerator::visitNamedTypeNode(NamedTypeNode* node) { (void)node; }
void CodeGenerator::visitArrayTypeNode(ArrayTypeNode* node) { (void)node; }
void CodeGenerator::visitRecordTypeNode(RecordTypeNode* node) { (void)node; }
void CodeGenerator::visitRangeNode(RangeNode* node) { (void)node; }
void CodeGenerator::visitEnumNode(EnumNode* node) { (void)node; }

void CodeGenerator::visitAssignNode(AssignNode* node) {
    if (node->value) node->value->accept(this);

    if (auto* var = dynamic_cast<VarNode*>(node->target)) {
        if (var->tabIndex >= 0 && var->lev >= 0) {
            int diff = currentLevel - var->lev;
            emit(Instruction(PCodeOp::STO, diff, var->tabIndex));
            return;
        }

        SymbolInfo* info = symbolTable->lookup(var->name);
        if (info) {
            int diff = currentLevel - info->level;
            emit(Instruction(PCodeOp::STO, diff, info->tabIndex));
            return;
        }
    }

    isLValueMode = true;
    if (node->target) node->target->accept(this);
    isLValueMode = false;
}

void CodeGenerator::visitIfNode(IfNode* node) {
    if (!node->condition) return;

    node->condition->accept(this);

    int jpcIndex = static_cast<int>(instructions.size());
    emit(Instruction(PCodeOp::JPC, 0, 0));

    if (node->thenBlock) node->thenBlock->accept(this);

    int jmpIndex = -1;
    if (node->elseBlock) {
        jmpIndex = static_cast<int>(instructions.size());
        emit(Instruction(PCodeOp::JMP, 0, 0));
    }

    int elseAddress = static_cast<int>(instructions.size());
    instructions[jpcIndex].value = elseAddress;

    if (node->elseBlock) {
        node->elseBlock->accept(this);
        int endAddress = static_cast<int>(instructions.size());
        instructions[jmpIndex].value = endAddress;
    }
}

void CodeGenerator::visitCaseBlockNode(CaseBlockNode* node) { if (node->statement) node->statement->accept(this); }
void CodeGenerator::visitCaseNode(CaseNode* node) { (void)node; }

void CodeGenerator::visitWhileNode(WhileNode* node) {
    int startAddress = static_cast<int>(instructions.size());

    if (node->condition) node->condition->accept(this);

    int jpcIndex = static_cast<int>(instructions.size());
    emit(Instruction(PCodeOp::JPC, 0, 0));

    if (node->loopBlock) node->loopBlock->accept(this);

    emit(Instruction(PCodeOp::JMP, 0, startAddress));

    int endAddress = static_cast<int>(instructions.size());
    instructions[jpcIndex].value = endAddress;
}

void CodeGenerator::visitRepeatNode(RepeatNode* node) {
    int startAddress = static_cast<int>(instructions.size());

    for (auto* stmt : node->statements) if (stmt) stmt->accept(this);
    if (node->condition) node->condition->accept(this);
    emit(Instruction(PCodeOp::JPC, 0, startAddress));
}

void CodeGenerator::visitForNode(ForNode* node) {
    if (!node->startValue || !node->endValue) return;
    node->startValue->accept(this);
    SymbolInfo* iterInfo = symbolTable->lookup(node->iteratorName);
    if (iterInfo) {
        int diff = currentLevel - iterInfo->level;
        emit(Instruction(PCodeOp::STO, diff, iterInfo->tabIndex));
    }

    int startAddress = static_cast<int>(instructions.size());

    if (iterInfo) emit(Instruction(PCodeOp::LOD, currentLevel - iterInfo->level, iterInfo->tabIndex));
    node->endValue->accept(this);

    if (!node->isDownto) {
        emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(OprCode::LEQ)));
    } else {
        emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(OprCode::GEQ)));
    }

    int jpcIndex = static_cast<int>(instructions.size());
    emit(Instruction(PCodeOp::JPC, 0, 0));

    if (node->loopBlock) node->loopBlock->accept(this);

    if (iterInfo) {
        emit(Instruction(PCodeOp::LOD, currentLevel - iterInfo->level, iterInfo->tabIndex));
        emit(Instruction(PCodeOp::LIT, 0, 1));
        if (!node->isDownto) emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(OprCode::ADD)));
        else emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(OprCode::SUB)));
        emit(Instruction(PCodeOp::STO, currentLevel - iterInfo->level, iterInfo->tabIndex));
    }

    emit(Instruction(PCodeOp::JMP, 0, startAddress));

    int endAddress = static_cast<int>(instructions.size());
    instructions[jpcIndex].value = endAddress;
}

void CodeGenerator::visitProcCallNode(ProcCallNode* node) {
    if (node->procName == "write" || node->procName == "writeln") {
        for (auto* arg : node->arguments) {
            if (!arg) continue;
            if (auto* stringNode = dynamic_cast<StringNode*>(arg)) {
                emit(Instruction(PCodeOp::OPR, stripQuotedLiteral(stringNode->value)));
                continue;
            }
            if (auto* charNode = dynamic_cast<CharNode*>(arg)) {
                emit(Instruction(PCodeOp::OPR, stripQuotedLiteral(charNode->value)));
                continue;
            }

            arg->accept(this);
            emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(OprCode::WRT)));
        }
        if (node->procName == "writeln") {
            emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(OprCode::WRTLN)));
        }
        return;
    }

    for (auto* arg : node->arguments) {
        if (!arg) continue;
        arg->accept(this);
    }
    SymbolInfo* info = symbolTable->lookup(node->procName);
    if (info && info->entryAddress >= 0) {
        int diff = currentLevel - info->level;
        emit(Instruction(PCodeOp::CAL, diff, info->entryAddress));
    } else {
        emit(Instruction(PCodeOp::CAL, 0, 0));
    }
}

void CodeGenerator::visitVarNode(VarNode* node) {
    if (node->tabIndex >= 0 && node->lev >= 0) {
        if (node->name == "true") {
            emit(Instruction(PCodeOp::LIT, 0, 1));
            return;
        }
        if (node->name == "false") {
            emit(Instruction(PCodeOp::LIT, 0, 0));
            return;
        }

        int diff = currentLevel - node->lev;
        emit(Instruction(PCodeOp::LOD, diff, node->tabIndex));
        return;
    }

    SymbolInfo* info = symbolTable->lookup(node->name);
    if (info) {
        if (info->kind == SymbolKind::CONSTANT && info->type == ExprType::BOOLEAN) {
            if (node->name == "true") {
                emit(Instruction(PCodeOp::LIT, 0, 1));
                return;
            }
            if (node->name == "false") {
                emit(Instruction(PCodeOp::LIT, 0, 0));
                return;
            }
        }

        int diff = currentLevel - info->level;
        emit(Instruction(PCodeOp::LOD, diff, info->tabIndex));
    }
}

void CodeGenerator::visitArrayAccessNode(ArrayAccessNode* node) {
    if (isLValueMode) {
        for (auto* idx : node->indices) if (idx) idx->accept(this);
        return;
    }

    if (node->arrayVar) node->arrayVar->accept(this);
    for (auto* idx : node->indices) if (idx) idx->accept(this);
}

void CodeGenerator::visitRecordAccessNode(RecordAccessNode* node) {
    if (isLValueMode) {
        return;
    }

    if (node->recordVar) node->recordVar->accept(this);
}

void CodeGenerator::visitBinOpNode(BinOpNode* node) {
    if (node->left) node->left->accept(this);
    if (node->right) node->right->accept(this);

    emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(mapBinaryOpr(node->op))));
}

void CodeGenerator::visitUnaryOpNode(UnaryOpNode* node) {
    if (!node->operand) return;

    if (node->op == "not") {
        node->operand->accept(this);
        emit(Instruction(PCodeOp::LIT, 0, 0));
        emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(OprCode::EQL)));
        return;
    }

    node->operand->accept(this);
    emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(mapUnaryOpr(node->op))));
}

void CodeGenerator::visitNumberNode(NumberNode* node) {
    int value = 0;
    try { value = std::stoi(node->value); } catch(...) {}
    emit(Instruction(PCodeOp::LIT, 0, value));
    (void)node;
}

void CodeGenerator::visitStringNode(StringNode* node) { (void)node; }
void CodeGenerator::visitCharNode(CharNode* node) { (void)node; }
