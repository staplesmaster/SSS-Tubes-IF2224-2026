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
    if (op == "and") return OprCode::AND;
    if (op == "or")  return OprCode::OR;
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
            for (const std::string& name : v->varNames) {
                SymbolInfo* info = symbolTable->lookup(name);
                varCount += info ? info->width : 1;
            }
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
        if (auto* v = dynamic_cast<VarDeclNode*>(decl)) {
            for (const std::string& name : v->varNames) {
                SymbolInfo* info = symbolTable->lookup(name);
                localVarCount += info ? info->width : 1;
            }
        }
    }
    int paramCount = 0;
    for (auto* p : node->parameters) if (auto* pn = dynamic_cast<ParamNode*>(p)) paramCount += static_cast<int>(pn->paramNames.size());

    emit(Instruction(PCodeOp::INT, 0, 3 + localVarCount));

    std::vector<const SymbolInfo*> pushableParams;
    for (auto* p : node->parameters) {
        if (auto* pn = dynamic_cast<ParamNode*>(p)) {
            for (const auto& name : pn->paramNames) {
                const SymbolInfo* found = nullptr;
                for (const auto& sym : symbolTable->allSymbols()) {
                    if (sym.name == name && sym.isParameter) {
                        found = &sym;
                        break;
                    }
                }
                if (found && found->type != ExprType::STRING) {
                    pushableParams.push_back(found);
                }
            }
        }
    }
    for (int i = static_cast<int>(pushableParams.size()) - 1; i >= 0; --i) {
        emit(Instruction(PCodeOp::STO, 0, pushableParams[i]->tabIndex));
    }

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
        SymbolInfo* info = symbolTable->lookup(var->name);
        int width = info ? info->width : 1;
        int baseTabIndex = var->tabIndex >= 0 ? var->tabIndex : (info ? info->tabIndex : -1);
        int lev = var->lev >= 0 ? var->lev : (info ? info->level : -1);

        if (baseTabIndex >= 0 && lev >= 0) {
            int diff = currentLevel - lev;
            for (int i = width - 1; i >= 0; --i) {
                emit(Instruction(PCodeOp::STO, diff, baseTabIndex + i));
            }
            return;
        }
    }

    isLValueMode = true;
    if (node->target) node->target->accept(this);
    isLValueMode = false;
    emit(Instruction(PCodeOp::STOI, 0, 0));
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
            if (arg->exprType == ExprType::BOOLEAN) {
                emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(OprCode::WRTBOOL)));
            } else {
                emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(OprCode::WRT)));
            }
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

        if (info->kind == SymbolKind::FUNCTION) {
            emit(Instruction(PCodeOp::LIT, 0, 0));
            emit(Instruction(PCodeOp::STO, diff, info->tabIndex));
        }

        emit(Instruction(PCodeOp::CAL, diff, info->entryAddress));

        if (info->kind == SymbolKind::FUNCTION) {
            emit(Instruction(PCodeOp::LOD, diff, info->tabIndex));
        }
    } else {
        emit(Instruction(PCodeOp::CAL, 0, 0));
    }
}

void CodeGenerator::visitVarNode(VarNode* node) {
    SymbolInfo* info = symbolTable->lookup(node->name);
    if (info) {
        if (info->kind == SymbolKind::CONSTANT && info->hasConstValue) {
            emit(Instruction(PCodeOp::LIT, 0, info->constValue));
            return;
        }
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
    }

    int tabIdx = node->tabIndex >= 0 ? node->tabIndex : (info ? info->tabIndex : -1);
    int lev    = node->lev    >= 0 ? node->lev    : (info ? info->level   : -1);

    if (tabIdx >= 0 && lev >= 0) {
        if (node->name == "true")  { emit(Instruction(PCodeOp::LIT, 0, 1)); return; }
        if (node->name == "false") { emit(Instruction(PCodeOp::LIT, 0, 0)); return; }

        int diff = currentLevel - lev;
        if (isAddressMode) {
            emit(Instruction(PCodeOp::LODA, diff, tabIdx));
        } else {
            emit(Instruction(PCodeOp::LOD, diff, tabIdx));
        }
    }
}

void CodeGenerator::visitArrayAccessNode(ArrayAccessNode* node) {
    SymbolInfo* info = nullptr;
    if (auto* v = dynamic_cast<VarNode*>(node->arrayVar)) {
        info = symbolTable->lookup(v->name);
    }
    
    if (info) {
        int diff = currentLevel - info->level;
        emit(Instruction(PCodeOp::LODA, diff, info->tabIndex));

        const ArrayInfo* arrInfo = (info->arrayIndex >= 0) ? symbolTable->getArray(info->arrayIndex) : nullptr;
        int lowerBound = 0;
        int upperBound = 0;
        int elemSize = 1;
        bool hasBounds = false;
        
        if (arrInfo) {
            lowerBound = arrInfo->lowerBound;
            upperBound = arrInfo->upperBound;
            elemSize = arrInfo->elemWidth;
            hasBounds = arrInfo->hasStaticBounds;
        }

        for (auto* idx : node->indices) {
            if (idx) idx->accept(this);
            
            if (hasBounds) {
                emit(Instruction(PCodeOp::CHK, lowerBound, upperBound));
            }
            
            emit(Instruction(PCodeOp::LIT, 0, lowerBound));
            emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(OprCode::SUB)));
            if (elemSize > 1) {
                emit(Instruction(PCodeOp::LIT, 0, elemSize));
                emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(OprCode::MUL)));
            }
            emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(OprCode::ADD)));
        }
        
        if (!isLValueMode) {
            emit(Instruction(PCodeOp::LODI, 0, 0));
        }
    }
}

void CodeGenerator::visitRecordAccessNode(RecordAccessNode* node) {
    bool prevAddressMode = isAddressMode;
    bool prevLValue      = isLValueMode;
    isAddressMode = true;
    isLValueMode  = false;
    if (node->recordVar) node->recordVar->accept(this);
    isAddressMode = prevAddressMode;
    isLValueMode  = prevLValue;

    emit(Instruction(PCodeOp::LIT, 0, node->fieldOffset));
    emit(Instruction(PCodeOp::OPR, 0, static_cast<int>(OprCode::ADD)));

    if (!isLValueMode) {
        emit(Instruction(PCodeOp::LODI, 0, 0));
    }
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
