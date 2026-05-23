#include "SemanticAnalyzer.hpp"
#include "TypeRules.hpp"

#include <cstdlib>

void SemanticAnalyzer::analyze(ASTNode* root) {
    if (!root) return; 
    root->accept(this);
}

const ErrorReporter& SemanticAnalyzer::getErrorReporter() const {
    return errorReporter;
}

const SymbolTable& SemanticAnalyzer::getSymbolTable() const {
    return symbolTable;
}

void SemanticAnalyzer::report(ASTNode* node, const std::string& message) {
    int line = node ? node->lineNum : 0; 
    errorReporter.addErrors(line, message);
}

ExprType SemanticAnalyzer::resolveTypeNode(ASTNode* typeNode){
    if (!typeNode) return ExprType::UNKNOWN;

    if (auto* namedType = dynamic_cast<NamedTypeNode*>(typeNode)) {
        if (namedType->typeName == "integer") return ExprType::INTEGER;
        if (namedType->typeName == "real") return ExprType::REAL;
        if (namedType->typeName == "boolean") return ExprType::BOOLEAN;
        if (namedType->typeName == "char") return ExprType::CHAR;
        if (namedType->typeName == "string") return ExprType::STRING;

        SymbolInfo* typeInfo = symbolTable.lookup(namedType->typeName);
        if (typeInfo && typeInfo->kind == SymbolKind::TYPE) {
            typeInfo->isUsed = true;
            if (typeInfo->type != ExprType::UNKNOWN) {
                return typeInfo->type;
            }
            if (typeInfo->typeDef && typeInfo->typeDef != typeNode) {
                ExprType resolved = resolveTypeNode(typeInfo->typeDef);
                typeInfo->type = resolved;
                return resolved;
            }
        }
        return ExprType::UNKNOWN;
    }

    if (dynamic_cast<ArrayTypeNode*>(typeNode)) return ExprType::ARRAY;

    if (dynamic_cast<RecordTypeNode*>(typeNode)) return ExprType::RECORD;

    if (dynamic_cast<RangeNode*>(typeNode)) return ExprType::SUBRANGE;

    if (dynamic_cast<EnumNode*>(typeNode)) return ExprType::ENUM;

    return ExprType::UNKNOWN;
}
int SemanticAnalyzer::getEnumDomainIdForTypeNode(ASTNode* typeNode) {
    if (!typeNode) return -1;

    if (auto* namedType = dynamic_cast<NamedTypeNode*>(typeNode)) {
        SymbolInfo* typeInfo = symbolTable.lookup(namedType->typeName);
        if (typeInfo && typeInfo->kind == SymbolKind::TYPE) {
            return typeInfo->enumDomainId;
        }
        return -1;
    }

    if (dynamic_cast<EnumNode*>(typeNode)) {
        auto it = enumDomainIds.find(typeNode);
        if (it != enumDomainIds.end()) return it->second;

        int domainId = nextEnumDomainId++;
        enumDomainIds[typeNode] = domainId;
        return domainId;
    }

    return -1;
}

int SemanticAnalyzer::getEnumDomainIdForExpression(ASTNode* expr) {
    if (!expr || expr->exprType != ExprType::ENUM) return -1;
    if (expr->enumDomainId != -1) return expr->enumDomainId;
    return getEnumDomainIdForTypeNode(getTypeDefForExpression(expr));
}

bool SemanticAnalyzer::areTypeNodesCompatible(ASTNode* leftTypeNode, ASTNode* rightTypeNode) {
    if (!leftTypeNode || !rightTypeNode) return false;

    if (auto* leftNamed = dynamic_cast<NamedTypeNode*>(leftTypeNode)) {
        SymbolInfo* info = symbolTable.lookup(leftNamed->typeName);
        if (info && info->kind == SymbolKind::TYPE && info->typeDef) {
            leftTypeNode = info->typeDef;
        }
    }

    if (auto* rightNamed = dynamic_cast<NamedTypeNode*>(rightTypeNode)) {
        SymbolInfo* info = symbolTable.lookup(rightNamed->typeName);
        if (info && info->kind == SymbolKind::TYPE && info->typeDef) {
            rightTypeNode = info->typeDef;
        }
    }

    ExprType leftType = resolveTypeNode(leftTypeNode);
    ExprType rightType = resolveTypeNode(rightTypeNode);

    if (leftType == ExprType::ENUM || rightType == ExprType::ENUM) {
        return leftType == rightType &&
               getEnumDomainIdForTypeNode(leftTypeNode) == getEnumDomainIdForTypeNode(rightTypeNode);
    }

    if (leftType == ExprType::ARRAY || rightType == ExprType::ARRAY) {
        auto* leftArray = dynamic_cast<ArrayTypeNode*>(leftTypeNode);
        auto* rightArray = dynamic_cast<ArrayTypeNode*>(rightTypeNode);
        return leftArray && rightArray &&
               areTypeNodesCompatible(leftArray->indexType, rightArray->indexType) &&
               areTypeNodesCompatible(leftArray->elementType, rightArray->elementType);
    }

    if (leftType == ExprType::RECORD || rightType == ExprType::RECORD) {
        auto* leftRecord = dynamic_cast<RecordTypeNode*>(leftTypeNode);
        auto* rightRecord = dynamic_cast<RecordTypeNode*>(rightTypeNode);
        if (!leftRecord || !rightRecord || leftRecord->fields.size() != rightRecord->fields.size()) {
            return false;
        }

        for (std::size_t i = 0; i < leftRecord->fields.size(); ++i) {
            auto* leftField = dynamic_cast<VarDeclNode*>(leftRecord->fields[i]);
            auto* rightField = dynamic_cast<VarDeclNode*>(rightRecord->fields[i]);
            if (!leftField || !rightField) return false;
            if (leftField->getVarNames() != rightField->getVarNames()) return false;
            if (!areTypeNodesCompatible(leftField->typeDef, rightField->typeDef)) return false;
        }
        return true;
    }

    if (leftType == ExprType::SUBRANGE || rightType == ExprType::SUBRANGE) {
        auto* leftRange = dynamic_cast<RangeNode*>(leftTypeNode);
        auto* rightRange = dynamic_cast<RangeNode*>(rightTypeNode);
        ExprType leftBase = leftRange && leftRange->lowerBound ? leftRange->lowerBound->exprType : leftType;
        ExprType rightBase = rightRange && rightRange->lowerBound ? rightRange->lowerBound->exprType : rightType;
        return TypeRules::isAssignable(leftBase, rightBase) && TypeRules::isAssignable(rightBase, leftBase);
    }

    return TypeRules::isAssignable(leftType, rightType) || TypeRules::isAssignable(rightType, leftType);
}

bool SemanticAnalyzer::tryGetIntLiteral(ASTNode* node, int& value) {
    if (!node) return false;
    if (auto* numberNode = dynamic_cast<NumberNode*>(node)) {
        if (numberNode->isReal) return false;
        char* endPtr = nullptr;
        long parsed = std::strtol(numberNode->value.c_str(), &endPtr, 10);
        if (endPtr && *endPtr == '\0') {
            value = static_cast<int>(parsed);
            return true;
        }
    }
    return false;
}

ExprType SemanticAnalyzer::resolveRecordFieldType(ASTNode* typeNode, const std::string& fieldName) {
    if (!typeNode) return ExprType::UNKNOWN;

    if (auto* namedType = dynamic_cast<NamedTypeNode*>(typeNode)) {
        SymbolInfo* typeInfo = symbolTable.lookup(namedType->typeName);
        if (typeInfo && typeInfo->kind == SymbolKind::TYPE) {
            typeInfo->isUsed = true;
            return resolveRecordFieldType(typeInfo->typeDef, fieldName);
        }
        return ExprType::UNKNOWN;
    }

    if (auto* recordType = dynamic_cast<RecordTypeNode*>(typeNode)) {
        for (ASTNode* fieldNode : recordType->fields) {
            auto* fieldDecl = dynamic_cast<VarDeclNode*>(fieldNode);
            if (!fieldDecl) continue;
            for (const std::string& name : fieldDecl->getVarNames()) {
                if (name == fieldName) {
                    return resolveTypeNode(fieldDecl->typeDef);
                }
            }
        }
    }

    return ExprType::UNKNOWN;
}

ASTNode* SemanticAnalyzer::getTypeDefForExpression(ASTNode* expr) {
    if (!expr) return nullptr;
    if (auto* varNode = dynamic_cast<VarNode*>(expr)) {
        SymbolInfo* info = symbolTable.lookup(varNode->name);
        return info ? info->typeDef : nullptr;
    }
    if (auto* arrayAcc = dynamic_cast<ArrayAccessNode*>(expr)) {
        ASTNode* parentTypeDef = getTypeDefForExpression(arrayAcc->arrayVar);
        if (auto* named = dynamic_cast<NamedTypeNode*>(parentTypeDef)) {
            SymbolInfo* info = symbolTable.lookup(named->typeName);
            parentTypeDef = info ? info->typeDef : nullptr;
        }
        if (auto* arrType = dynamic_cast<ArrayTypeNode*>(parentTypeDef)) {
            return arrType->elementType;
        }
    }
    if (auto* recAcc = dynamic_cast<RecordAccessNode*>(expr)) {
        ASTNode* parentTypeDef = getTypeDefForExpression(recAcc->recordVar);
        if (auto* named = dynamic_cast<NamedTypeNode*>(parentTypeDef)) {
            SymbolInfo* info = symbolTable.lookup(named->typeName);
            parentTypeDef = info ? info->typeDef : nullptr;
        }
        if (auto* recType = dynamic_cast<RecordTypeNode*>(parentTypeDef)) {
            for (ASTNode* fieldNode : recType->fields) {
                if (auto* fieldDecl = dynamic_cast<VarDeclNode*>(fieldNode)) {
                    for (const std::string& name : fieldDecl->getVarNames()) {
                        if (name == recAcc->fieldName) return fieldDecl->typeDef;
                    }
                }
            }
        }
    }
    return nullptr;
}

void SemanticAnalyzer::visitProgramNode(ProgramNode* node) {
    if (!node) return;

    node->lev = symbolTable.currentLevel();

    SymbolInfo programInfo(node->getProgramName(), SymbolKind::PROGRAM);
    programInfo.type = ExprType::VOID;
    programInfo.declLine = node->lineNum;

    symbolTable.declare(node->getProgramName(), programInfo);

    SymbolInfo* declaredProgram = symbolTable.lookUpCurrent(node->getProgramName());
    int programBlock = symbolTable.createBlock(
        node->getProgramName(),
        -1,
        symbolTable.currentLevel(),
        symbolTable.nextTabIndex()
    );
    if (declaredProgram) {
        declaredProgram->blockIndex = programBlock;
    }
    activeBlocks.push_back(programBlock);

    for (ASTNode* decl : node->declarations) {
        if (decl) decl->accept(this);
    }

    if (node->mainBlock) {
        symbolTable.enterScope();
        node->mainBlock->accept(this);
        symbolTable.exitScope();
    }

    symbolTable.closeBlock(programBlock, symbolTable.nextTabIndex() - 1);
    activeBlocks.pop_back();
}

void SemanticAnalyzer::visitCompoundNode(CompoundNode* node) {
    if (!node) return;

    node->lev = symbolTable.currentLevel();

    for (ASTNode* stmt : node->statements) {
        if (stmt) stmt->accept(this);
    }
}

void SemanticAnalyzer::visitConstDeclNode(ConstDeclNode* node) {
    if (!node) return;

    if (node->value){
        node->value->accept(this);
    }

    SymbolInfo info(node->getConstName(), SymbolKind::CONSTANT);
    info.type = node->value ? node->value->exprType : ExprType::UNKNOWN;
    info.enumDomainId = node->value ? getEnumDomainIdForExpression(node->value) : -1;
    info.declLine = node->lineNum;
    info.blockIndex = activeBlocks.empty() ? -1 : activeBlocks.back();

    if (!symbolTable.declare(node->getConstName(), info)) {
        report(node, "Deklarasi ulang konstanta '" + node->getConstName() + "'");
    }
}

void SemanticAnalyzer::visitTypeDeclNode(TypeDeclNode* node) {
    if (!node) return;

    if (node->typeDef){
        node->typeDef->accept(this);
    }

    SymbolInfo info(node->getTypeName(), SymbolKind::TYPE);
    info.type = resolveTypeNode(node->typeDef);
    info.typeDef = node->typeDef;
    info.enumDomainId = getEnumDomainIdForTypeNode(node->typeDef);
    info.declLine = node->lineNum;
    info.blockIndex = activeBlocks.empty() ? -1 : activeBlocks.back();

    if (!symbolTable.declare(node->getTypeName(), info)) {
        report(node, "Deklarasi ulang tipe '" + node->getTypeName() + "'");
    }
}

void SemanticAnalyzer::visitVarDeclNode(VarDeclNode* node) {
    if (!node) return;

    if (node->typeDef){
        node->typeDef->accept(this);
    }

    ExprType varType = resolveTypeNode(node->typeDef);
    int arrayInfoIndex = -1;

    if (auto* arrayType = dynamic_cast<ArrayTypeNode*>(node->typeDef)) {
        ExprType indexType = resolveTypeNode(arrayType->indexType);
        ExprType elementType = resolveTypeNode(arrayType->elementType);

        bool hasStaticBounds = false;
        int lowerBound = 0;
        int upperBound = 0;
        if (auto* rangeType = dynamic_cast<RangeNode*>(arrayType->indexType)) {
            int low = 0;
            int high = 0;
            if (tryGetIntLiteral(rangeType->lowerBound, low) && tryGetIntLiteral(rangeType->upperBound, high)) {
                hasStaticBounds = true;
                lowerBound = low;
                upperBound = high;
            }
        }

        arrayInfoIndex = symbolTable.createArray(
            indexType,
            elementType,
            arrayType->indexType,
            arrayType->elementType,
            hasStaticBounds,
            lowerBound,
            upperBound
        );
    }

    for (const std::string& name : node->getVarNames()){
        SymbolInfo info(name, SymbolKind::VARIABLE);
        info.type = varType;
        info.declLine = node->lineNum;
        info.typeDef = node->typeDef;
        info.enumDomainId = getEnumDomainIdForTypeNode(node->typeDef);
        info.blockIndex = activeBlocks.empty() ? -1 : activeBlocks.back();

        if (!symbolTable.declare(name, info)) {
            report(node, "Deklarasi ulang variabel '" + name + "'");
            continue;
        }

        SymbolInfo* declaredInfo = symbolTable.lookUpCurrent(name);
        if (declaredInfo) {
            declaredInfo->arrayIndex = arrayInfoIndex;
            node->tabIndex = declaredInfo->tabIndex;
            node->lev = declaredInfo->level;
            node->exprType = varType;   
        }
    }
}

void SemanticAnalyzer::visitParamNode(ParamNode* node) {
    if (!node) return;

    if (node->typeDef){
        node->typeDef->accept(this);
    }

    ExprType paramType = resolveTypeNode(node->typeDef);

    for (const std::string& name : node->paramNames){
        SymbolInfo info(name, SymbolKind::PARAMETER);
        info.type = paramType;
        info.declLine = node->lineNum;
        info.isParameter = true;
        info.enumDomainId = getEnumDomainIdForTypeNode(node->typeDef);
        info.blockIndex = activeBlocks.empty() ? -1 : activeBlocks.back();

        if (!symbolTable.declare(name, info)) {
            report(node, "Deklarasi ulang parameter '" + name + "'");
            continue;
        }
    }
}

void SemanticAnalyzer::visitSubprogramDeclNode(SubprogramDeclNode* node) {
    if (!node) return;

    SymbolKind kind = node->isFunction ? SymbolKind::FUNCTION : SymbolKind::PROCEDURE;
    SymbolInfo info(node->getSubprogramName(), kind);
    info.type = node->isFunction ? resolveTypeNode(node->returnType) : ExprType::VOID;
    info.blockIndex = activeBlocks.empty() ? -1 : activeBlocks.back();

    if (!symbolTable.declare(node->getSubprogramName(), info)) {
        report(node, "Deklarasi ulang subprogram '" + node->getSubprogramName() + "'");
        return;
    }

    SymbolInfo* declaredSub = symbolTable.lookUpCurrent(node->getSubprogramName());
    int parentBlock = activeBlocks.empty() ? -1 : activeBlocks.back();
    int newBlock = symbolTable.createBlock(
        node->getSubprogramName(),
        parentBlock,
        symbolTable.currentLevel() + 1,
        symbolTable.nextTabIndex()
    );
    if (declaredSub) declaredSub->blockIndex = newBlock;

    if (declaredSub) {
        declaredSub->parameters.clear();
        for (ASTNode* paramNode : node->parameters) {
            auto* paramDecl = dynamic_cast<ParamNode*>(paramNode);
            if (!paramDecl) continue;
            ExprType pType = resolveTypeNode(paramDecl->typeDef);
            for (const std::string& pName : paramDecl->paramNames) {
                ParamInfo pInfo;
                pInfo.name = pName;
                pInfo.type = pType;
                pInfo.enumDomainId = getEnumDomainIdForTypeNode(paramDecl->typeDef);
                pInfo.isVarParam = paramDecl->isVarParam;
                pInfo.declLine = paramDecl->lineNum;
                declaredSub->parameters.push_back(pInfo);
            }
        }
    }

    activeBlocks.push_back(newBlock);
    symbolTable.enterScope();

    for (ASTNode* param : node->parameters) {
        if (param) param->accept(this);
    }

    for (ASTNode* decl : node->declarations) {
        if (decl) decl->accept(this);
    }

    if (node->body) {
        node->body->accept(this);
    }
    
    symbolTable.closeBlock(newBlock, symbolTable.nextTabIndex() - 1);
    symbolTable.exitScope();
    activeBlocks.pop_back();
}

void SemanticAnalyzer::visitNamedTypeNode(NamedTypeNode* node) {
    if (!node) return;
    node->exprType = resolveTypeNode(node);
}

void SemanticAnalyzer::visitArrayTypeNode(ArrayTypeNode* node) {
    if (!node) return;
 
    if(node->indexType) node->indexType->accept(this);
    if(node->elementType) node->elementType->accept(this);

    ExprType indexType = resolveTypeNode(node->indexType);
    if (indexType == ExprType::REAL) {
        report(node->indexType, "Tipe indeks array tidak boleh real");
    } else if (indexType != ExprType::UNKNOWN &&
               indexType != ExprType::SUBRANGE &&
               !TypeRules::isValidArrayIndexType(indexType)) {
        report(node->indexType, "Tipe indeks array harus simple type dan bukan real");
    }

    node->exprType = ExprType::ARRAY;
}

void SemanticAnalyzer::visitRecordTypeNode(RecordTypeNode* node) {
    if (!node) return;

    for (ASTNode* field : node->fields) {
        if (field) field->accept(this);
    }

    node->exprType = ExprType::RECORD;
}

void SemanticAnalyzer::visitRangeNode(RangeNode* node) {
    if (!node) return;

    if(node->lowerBound) node->lowerBound->accept(this);
    if(node->upperBound) node->upperBound->accept(this);

    ExprType lowerType = node->lowerBound ? node->lowerBound->exprType : ExprType::UNKNOWN;
    ExprType upperType = node->upperBound ? node->upperBound->exprType : ExprType::UNKNOWN;

    if (lowerType == ExprType::REAL || upperType == ExprType::REAL) {
        report(node, "Subrange tidak boleh bertipe real");
    } else if (lowerType != ExprType::UNKNOWN && upperType != ExprType::UNKNOWN) {
        if (!TypeRules::isAssignable(lowerType, upperType) || !TypeRules::isAssignable(upperType, lowerType)) {
            report(node, "Batas bawah dan batas atas subrange harus memiliki tipe compatible");
        }
    }

    int lowerValue = 0;
    int upperValue = 0;
    if (tryGetIntLiteral(node->lowerBound, lowerValue) && tryGetIntLiteral(node->upperBound, upperValue) &&
        lowerValue > upperValue) {
        report(node, "Lower bound subrange tidak boleh lebih besar dari upper bound");
    }

    auto* lowerChar = dynamic_cast<CharNode*>(node->lowerBound);
    auto* upperChar = dynamic_cast<CharNode*>(node->upperBound);
    if (lowerChar && upperChar && !lowerChar->value.empty() && !upperChar->value.empty() &&
        lowerChar->value[0] > upperChar->value[0]) {
        report(node, "Lower bound subrange tidak boleh lebih besar dari upper bound");
    }

    node->exprType = ExprType::SUBRANGE;
}

void SemanticAnalyzer::visitEnumNode(EnumNode* node) {
    if (!node) return;
    node->exprType = ExprType::ENUM;
    node->enumDomainId = getEnumDomainIdForTypeNode(node);

    for (const std::string& enumVal : node->identifiers) {
        SymbolInfo enumConst(enumVal, SymbolKind::CONSTANT);
        enumConst.type = ExprType::ENUM;
        enumConst.typeDef = node;
        enumConst.enumDomainId = node->enumDomainId;
        enumConst.declLine = node->lineNum;
        enumConst.blockIndex = activeBlocks.empty() ? -1 : activeBlocks.back();
        if (!symbolTable.declare(enumVal, enumConst)) {
            report(node, "Deklarasi ulang konstanta enum '" + enumVal + "'");
        }
    }
}

void SemanticAnalyzer::visitAssignNode(AssignNode* node) {
    if (!node) return;

    if(node->target) node->target->accept(this);
    if(node->value) node->value->accept(this);

    if(node->target && node->value) {
        ExprType targetType = node->target->exprType;
        ExprType valueType = node->value->exprType;

        if (targetType != ExprType::UNKNOWN && valueType != ExprType::UNKNOWN) {
            bool compatible = TypeRules::isAssignable(targetType, valueType);

            if ((targetType == ExprType::ARRAY || targetType == ExprType::RECORD ||
                 valueType == ExprType::ARRAY || valueType == ExprType::RECORD)) {
                compatible = areTypeNodesCompatible(
                    getTypeDefForExpression(node->target),
                    getTypeDefForExpression(node->value)
                );
            }

            if (!compatible) {
                report(node, "Tipe assignment tidak kompatible");
            } else if (targetType == ExprType::ENUM && valueType == ExprType::ENUM &&
                       getEnumDomainIdForExpression(node->target) != getEnumDomainIdForExpression(node->value)) {
                report(node, "Tipe assignment enum berasal dari domain berbeda");
            }
        }
    }

    node->exprType = ExprType::VOID;
}

void SemanticAnalyzer::visitIfNode(IfNode* node) {
    if (!node) return;

    if(node->condition) node->condition->accept(this);
    if (node->condition) {
        ExprType condType = node->condition->exprType;
        if (condType != ExprType::UNKNOWN &&
            condType != ExprType::BOOLEAN &&
            condType != ExprType::INTEGER) {
            report(node->condition, "Kondisi if harus bertipe boolean atau integer");
        }
    }
    if(node->thenBlock) node->thenBlock->accept(this);
    if(node->elseBlock) node->elseBlock->accept(this);
}

void SemanticAnalyzer::visitCaseBlockNode(CaseBlockNode* node) {
    if (!node) return;

    for (ASTNode* constant : node->constants) {
        if (constant) constant->accept(this);
    }

    if(node->statement) node->statement->accept(this);
}

void SemanticAnalyzer::visitCaseNode(CaseNode* node) {
    if (!node) return;

    if(node->condition) node->condition->accept(this);

    for (ASTNode* caseBlock : node->cases) {
        if (caseBlock) caseBlock->accept(this);
    }
}

void SemanticAnalyzer::visitWhileNode(WhileNode* node) {
    if (!node) return;

    if(node->condition) node->condition->accept(this);
    if (node->condition) {
        ExprType condType = node->condition->exprType;
        if (condType != ExprType::UNKNOWN &&
            condType != ExprType::BOOLEAN &&
            condType != ExprType::INTEGER) {
            report(node->condition, "Kondisi while harus bertipe boolean atau integer");
        }
    }
    if(node->loopBlock) node->loopBlock->accept(this);
}

void SemanticAnalyzer::visitRepeatNode(RepeatNode* node) {
    if (!node) return;

    for (ASTNode* stmt : node->statements) {
        if (stmt) stmt->accept(this);
    }

    if(node->condition) node->condition->accept(this);
    if (node->condition) {
        ExprType condType = node->condition->exprType;
        if (condType != ExprType::UNKNOWN &&
            condType != ExprType::BOOLEAN &&
            condType != ExprType::INTEGER) {
            report(node->condition, "Kondisi repeat-until harus bertipe boolean atau integer");
        }
    }
}

void SemanticAnalyzer::visitForNode(ForNode* node) {
    if (!node) return;

    SymbolInfo* iter = symbolTable.lookup(node->iteratorName);
    if (!iter) {
        report(node, "Iterator '" + node->iteratorName + "' belum dideklarasikan");
    } 
    if(node->startValue) node->startValue->accept(this);
    if(node->endValue) node->endValue->accept(this);
    if(node->loopBlock) node->loopBlock->accept(this);

    if (iter) {
        ExprType iterType = iter->type;
        if (iterType != ExprType::UNKNOWN) {
            if (!TypeRules::isInteger(iterType) && iterType != ExprType::SUBRANGE) {
                report(node, "Iterator harus bertipe integer atau subrange");
            }

            if (node->startValue && node->startValue->exprType != ExprType::UNKNOWN) {
                if (!TypeRules::isAssignable(iterType, node->startValue->exprType)) {
                    report(node->startValue, "Tipe awal tidak kompatibel dengan iterator");
                }
            }
            if (node->endValue && node->endValue->exprType != ExprType::UNKNOWN) {
                if (!TypeRules::isAssignable(iterType, node->endValue->exprType)) {
                    report(node->endValue, "Tipe akhir tidak kompatibel dengan iterator");
                }
            }
        }
    }
}

void SemanticAnalyzer::visitProcCallNode(ProcCallNode* node) {
    if (!node) return;

    std::vector<ASTNode*> args = node->getArguments();
    for (ASTNode* arg : args) {
        if (arg) arg->accept(this);
    }

    SymbolInfo* procInfo = symbolTable.lookup(node->procName);
    if (!procInfo) {
        report(node, "Subprogram '" + node->procName + "' belum dideklarasikan");
    } else {
        procInfo->isUsed = true;
        node->tabIndex = procInfo->tabIndex;
        node->lev = procInfo->level;

        if (procInfo->kind != SymbolKind::PROCEDURE && procInfo->kind != SymbolKind::FUNCTION) {
            report(node, "Identifier '" + node->procName + "' bukan subprogram");
        }

        const bool isBuiltinIo =
            node->procName == "writeln" ||
            node->procName == "write" ||
            node->procName == "readln" ||
            node->procName == "read";

        if (!isBuiltinIo) {
            const std::size_t expected = procInfo->parameters.size();
            const std::size_t actual = args.size();
            if (expected != actual) {
                report(node, "Jumlah argumen tidak sesuai untuk pemanggilan '" + node->procName + "'");
            }

            const std::size_t n = expected < actual ? expected : actual;
            for (std::size_t i = 0; i < n; ++i) {
                ASTNode* arg = args[i];
                ExprType argType = arg ? arg->exprType : ExprType::UNKNOWN;
                ExprType paramType = procInfo->parameters[i].type;
                if (argType != ExprType::UNKNOWN && paramType != ExprType::UNKNOWN && !TypeRules::isAssignable(paramType, argType)) {
                    report(arg, "Tipe argumen ke-" + std::to_string(i + 1) + " tidak kompatibel");
                } else if (argType == ExprType::ENUM && paramType == ExprType::ENUM &&
                           procInfo->parameters[i].enumDomainId != getEnumDomainIdForExpression(arg)) {
                    report(arg, "Tipe argumen enum ke-" + std::to_string(i + 1) + " berasal dari domain berbeda");
                }
            }
        }
    }

    if (procInfo) {
        node->exprType = procInfo->type;
    } else {
        node->exprType = ExprType::UNKNOWN;
    }
}

void SemanticAnalyzer::visitVarNode(VarNode* node) {
    if (!node) return;

    SymbolInfo* varInfo = symbolTable.lookup(node->name);
    if (!varInfo) {
        report(node, "Variabel '" + node->name + "' belum dideklarasikan");
        node->exprType = ExprType::UNKNOWN;
        return;
    } 
    varInfo->isUsed = true;
    node->tabIndex = varInfo->tabIndex;
    node->lev = varInfo->level;
    node->exprType = varInfo->type;
    node->enumDomainId = varInfo->enumDomainId;
}

void SemanticAnalyzer::visitArrayAccessNode(ArrayAccessNode* node) {
    if (!node) return;

    if(node->arrayVar) node->arrayVar->accept(this);

    for (ASTNode* idx : node->indices) {
        if (idx) idx->accept(this);
    }

    for (ASTNode* idx : node->indices) {
        if (!idx) continue;
        if (idx->exprType != ExprType::UNKNOWN && !TypeRules::isValidArrayIndexType(idx->exprType)) {
            report(idx, "Tipe indeks array tidak valid");
        }
    }

    node->exprType = ExprType::UNKNOWN;
    
    ASTNode* arrayTypeDef = getTypeDefForExpression(node->arrayVar);
    if (auto* named = dynamic_cast<NamedTypeNode*>(arrayTypeDef)) {
        SymbolInfo* info = symbolTable.lookup(named->typeName);
        if (info) arrayTypeDef = info->typeDef;
    }

    if (auto* arrType = dynamic_cast<ArrayTypeNode*>(arrayTypeDef)) {
        node->exprType = resolveTypeNode(arrType->elementType);
        node->enumDomainId = getEnumDomainIdForTypeNode(arrType->elementType);

        ExprType declaredIndexType = resolveTypeNode(arrType->indexType);
        if (auto* rangeType = dynamic_cast<RangeNode*>(arrType->indexType)) {
            declaredIndexType = rangeType->lowerBound ? rangeType->lowerBound->exprType : declaredIndexType;
        }

        for (ASTNode* idx : node->indices) {
            if (!idx || idx->exprType == ExprType::UNKNOWN || declaredIndexType == ExprType::UNKNOWN) continue;
            if (!TypeRules::isAssignable(declaredIndexType, idx->exprType)) {
                report(idx, "Tipe indeks tidak kompatibel dengan tipe indeks array");
            }
        }

        bool hasStaticBounds = false;
        int lowerBound = 0;
        int upperBound = 0;
        if (auto* rangeType = dynamic_cast<RangeNode*>(arrType->indexType)) {
            int low = 0, high = 0;
            if (tryGetIntLiteral(rangeType->lowerBound, low) && tryGetIntLiteral(rangeType->upperBound, high)) {
                hasStaticBounds = true;
                lowerBound = low;
                upperBound = high;
            }
        }

        if (hasStaticBounds && !node->indices.empty()) {
            int idxValue = 0;
            if (tryGetIntLiteral(node->indices[0], idxValue)) {
                if (idxValue < lowerBound || idxValue > upperBound) {
                    report(node->indices[0], "Index array di luar batas statis");
                }
            }
        }
    }
}

void SemanticAnalyzer::visitRecordAccessNode(RecordAccessNode* node) {
    if (!node) return;

    if(node->recordVar) node->recordVar->accept(this);

    node->exprType = ExprType::UNKNOWN;

    auto* varNode = dynamic_cast<VarNode*>(node->recordVar);
    if (!varNode) {
        return;
    }

    SymbolInfo* recInfo = symbolTable.lookup(varNode->name);
    if (!recInfo) {
        report(node, "Record variable belum dideklarasikan");
        return;
    }

    ExprType fieldType = resolveRecordFieldType(recInfo->typeDef, node->fieldName);
    if (fieldType == ExprType::UNKNOWN) {
        report(node, "Field '" + node->fieldName + "' tidak ditemukan pada record");
        return;
    }

    node->exprType = fieldType;
}

void SemanticAnalyzer::visitBinOpNode(BinOpNode* node) {
    if (!node) return;

    if(node->left) node->left->accept(this);
    if(node->right) node->right->accept(this);

    ExprType leftType = node->left ? node->left->exprType : ExprType::UNKNOWN;
    ExprType rightType = node->right ? node->right->exprType : ExprType::UNKNOWN;

    ExprType result = TypeRules::resultOfBinary(node->op, leftType, rightType);
    node->exprType = result;

    const bool isRelational =
        node->op == "=" || node->op == "==" || node->op == "<>" || node->op == "<" ||
        node->op == ">" || node->op == "<=" || node->op == ">=";

    if (isRelational && leftType == ExprType::ENUM && rightType == ExprType::ENUM &&
        getEnumDomainIdForExpression(node->left) != getEnumDomainIdForExpression(node->right)) {
        node->exprType = ExprType::UNKNOWN;
        report(node, "Ekspresi enum berasal dari domain berbeda");
        return;
    }

    if (result == ExprType::UNKNOWN && leftType != ExprType::UNKNOWN && rightType != ExprType::UNKNOWN) {
        report(node, "Ekspresi biner tidak kompatibel untuk operator '" + node->op + "'");
    }
}

void SemanticAnalyzer::visitUnaryOpNode(UnaryOpNode* node) {
    if (!node) return;

    if(node->operand) node->operand->accept(this);
    ExprType opd = node->operand ? node->operand->exprType : ExprType::UNKNOWN;
    ExprType res = TypeRules::resultOfUnary(node->op, opd);
    node->exprType = res;

    if (res == ExprType::UNKNOWN && opd != ExprType::UNKNOWN) {
        report(node, "Operator unary '" + node->op + "' tidak dapat diterapkan pada tipe ini");
    }
}

void SemanticAnalyzer::visitNumberNode(NumberNode* node) {
    if (!node) return;
    node->exprType = node->isReal ? ExprType::REAL : ExprType::INTEGER;
}

void SemanticAnalyzer::visitStringNode(StringNode* node) {
    if (!node) return;
    node->exprType = ExprType::STRING;
}

void SemanticAnalyzer::visitCharNode(CharNode* node) {
    if (!node) return;
    node->exprType = ExprType::CHAR;
}
