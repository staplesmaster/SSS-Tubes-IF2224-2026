#include "ASTConverter.hpp"
#include "Parser.hpp"

ASTNode* ASTConverter::build(ParseNode* parseRoot) {
    if (!parseRoot) return nullptr;
    
    if (parseRoot->getName() == "<program>") {
        return convertProgram(parseRoot);
    }
    return nullptr;
}

// STRUKTUR PROGRAM
/*
 * SDT: <program> -> <program-header> + <declaration-part> + <compound-statement> + period
 * Semantic Action: return new ProgramNode(header.name, declarations, mainBlock)
 */
ASTNode* ASTConverter::convertProgram(ParseNode* node) {
    // children[0] = <program-header>
    // children[1] = <declaration-part>
    // children[2] = <compound-statement>
    // children[3] = period (diabaikan)
    
    ParseNode* header = node->getChildren()[0];
    
    // <program-header> -> programsy + ident + semicolon
    // header.children[1] adalah ident (nama program)
    string progName = header->getChildren()[1]->getToken().value; 

    vector<ASTNode*> declarations = convertDeclarationPart(node->getChildren()[1]);
    ASTNode* mainBlock = convertCompoundStatement(node->getChildren()[2]);

    return new ProgramNode(progName, declarations, mainBlock);
}

/*
 * SDT: <compound-statement> -> beginsy + <statement-list> + endsy
 * Semantic Action: return new CompoundNode(statements)
 */
ASTNode* ASTConverter::convertCompoundStatement(ParseNode* node) {
    // children[0] = beginsy (diabaikan)
    // children[1] = <statement-list>
    // children[2] = endsy (diabaikan)

    vector<ASTNode*> statements = convertStatementList(node->getChildren()[1]);
    
    return new CompoundNode(statements);
}

/*
 * SDT: <statement-list> -> <statement> + (semicolon + <statement>)*
 * Semantic Action: Kumpulkan semua statement ke dalam vector, abaikan semicolon
 */
vector<ASTNode*> ASTConverter::convertStatementList(ParseNode* node) {
    vector<ASTNode*> stmts;
    
    // Jumlah statement dinamis (*)
    for (ParseNode* child : node->getChildren()) {
        if (child->getName() == "<statement>") {
            ASTNode* stmtNode = convertStatement(child);
            
            // Jika buka empty statement, masukkan
            if (stmtNode != nullptr) {
                stmts.push_back(stmtNode);
            }
        }
    }
    
    return stmts;
}


// DEKLARASI
/*
 * SDT: <declaration-part> -> (const)* + (type)* + (var)* + (subprogram)*
 * Semantic Action: Looping anak-anaknya, ekstrak node, gabungkan semua menjadi satu list deklarasi.
 */
vector<ASTNode*> ASTConverter::convertDeclarationPart(ParseNode* node) {
    vector<ASTNode*> allDecls;
    
    // Deklarasi bervariasi dan dinamis (*)
    for (ParseNode* child : node->getChildren()) {
        string childName = child->getName();
        
        if (childName == "<const-declaration>") {
            vector<ASTNode*> cDecls = convertConstDeclaration(child);
            allDecls.insert(allDecls.end(), cDecls.begin(), cDecls.end());
        } 
        else if (childName == "<type-declaration>") {
            vector<ASTNode*> tDecls = convertTypeDeclaration(child);
            allDecls.insert(allDecls.end(), tDecls.begin(), tDecls.end());
        } 
        else if (childName == "<var-declaration>") {
            vector<ASTNode*> vDecls = convertVarDeclaration(child);
            allDecls.insert(allDecls.end(), vDecls.begin(), vDecls.end());
        } 
        else if (childName == "<subprogram-declaration>") {
            allDecls.push_back(convertSubprogramDeclaration(child));
        }
    }
    return allDecls;
}

/*
 * SDT: <const-declaration> -> constsy + (ident + eql + constant + semicolon)+
 * Semantic Action: return new ConstDeclNode(ident.name, constant.node)
 */
vector<ASTNode*> ASTConverter::convertConstDeclaration(ParseNode* node) {
    vector<ASTNode*> result;
    // children[0] = constsy (diabaikan)
    // Berulang tiap 4 token: [1]=ident, [2]=eql, [3]=constant, [4]=semicolon
    
    for (size_t i = 1; i < node->getChildren().size(); i += 4) {
        if (i + 2 >= node->getChildren().size()) break;
        string name = node->getChildren()[i]->getToken().value;
        ASTNode* constVal = convertConstant(node->getChildren()[i+2]);
        
        result.push_back(new ConstDeclNode(name, constVal));
    }
    return result;
}

/*
 * SDT: <type-declaration> -> typesy + (ident + eql + type + semicolon)+
 * Semantic Action: return new TypeDeclNode(ident.name, type.node)
 */
vector<ASTNode*> ASTConverter::convertTypeDeclaration(ParseNode* node) {
    vector<ASTNode*> result;
    // children[0] = typesy (diabaikan)
    // Berulang tiap 4 token: [1]=ident, [2]=eql, [3]=type, [4]=semicolon
    
    for (size_t i = 1; i < node->getChildren().size(); i += 4) {
        if (i + 2 >= node->getChildren().size()) break;
        string name = node->getChildren()[i]->getToken().value;
        ASTNode* typeDef = convertType(node->getChildren()[i+2]);
        
        result.push_back(new TypeDeclNode(name, typeDef));
    }
    return result;
}

/*
 * SDT: <var-declaration> -> varsy + (identifier-list + colon + type + semicolon)+
 * Semantic Action: return new VarDeclNode(identifier_list.names, type.node)
 */
vector<ASTNode*> ASTConverter::convertVarDeclaration(ParseNode* node) {
    vector<ASTNode*> result;
    // children[0] = varsy (diabaikan)
    // Berulang tiap 4 token: [1]=identifier-list, [2]=colon, [3]=type, [4]=semicolon
    
    for (size_t i = 1; i < node->getChildren().size(); i += 4) {
        if (i + 2 >= node->getChildren().size()) break;
        vector<string> names = convertIdentifierList(node->getChildren()[i]);
        ASTNode* typeDef = convertType(node->getChildren()[i+2]);
        
        result.push_back(new VarDeclNode(names, typeDef));
    }
    return result;
}

/*
 * SDT: <identifier-list> -> ident + (comma + ident)*
 * Semantic Action: Ekstrak teks ident, abaikan comma, masukkan ke vector<string>
 */
vector<string> ASTConverter::convertIdentifierList(ParseNode* node) {
    vector<string> names;
    // [0]=ident, (Berulang tiap 2 token: [1]=comma, [2]=ident)
    for (size_t i = 0; i < node->getChildren().size(); i += 2) {
        names.push_back(node->getChildren()[i]->getToken().value);
    }
    return names;
}

/*
 * SDT: 
 * <subprogram-declaration> -> <procedure-declaration> | <function-declaration>
 * <procedure-declaration> -> proceduresy + ident + (formal-parameter-list)? + semicolon + block + semicolon
 * <function-declaration> -> functionsy + ident + (formal-parameter-list)? + colon + ident + semicolon + block + semicolon
 * Semantic Action: return new SubprogramDeclNode(isFunction, name, params, returnType, decls, block)
 */
ASTNode* ASTConverter::convertSubprogramDeclaration(ParseNode* node) {
    ParseNode* actualDecl = node->getChildren()[0];
    bool isFunc = (actualDecl->getName() == "<function-declaration>");
    
    // children[0] = proceduresy/functionsy
    // children[1] = ident (nama subprogram)
    string subName = actualDecl->getChildren()[1]->getToken().value;
    
    vector<ASTNode*> params;
    size_t idx = 2;
    
    if (idx < actualDecl->getChildren().size() && actualDecl->getChildren()[idx]->getName() == "<formal-parameter-list>") {
        params = convertFormalParameterList(actualDecl->getChildren()[idx]);
        idx++;
    }
    
    ASTNode* retType = nullptr;
    if (isFunc && idx < actualDecl->getChildren().size()) {
        idx++; // abaikan colon
        if (idx < actualDecl->getChildren().size()) {
            retType = new NamedTypeNode(actualDecl->getChildren()[idx]->getToken().value);
            idx++;
        }
    }
    
    idx++; // abaikan semicolon (;) pertama

    vector<ASTNode*> localDecls;
    ASTNode* bodyBlock = nullptr;

    if (idx < actualDecl->getChildren().size()) {
        // actualDecl.children[idx] adalah <block>
        ParseNode* blockNode = actualDecl->getChildren()[idx];
        
        // Ekstrak dari <block> -> <declaration-part> + <compound-statement>
        if (blockNode != nullptr && blockNode->getChildren().size() >= 2) {
            localDecls = convertDeclarationPart(blockNode->getChildren()[0]);
            bodyBlock = convertCompoundStatement(blockNode->getChildren()[1]);
        }
    }
    
    return new SubprogramDeclNode(isFunc, subName, params, retType, localDecls, bodyBlock);
}

/*
 * SDT: <formal-parameter-list> -> lparent + parameter-group + (semicolon + parameter-group)* + rparent
 * Semantic Action: Ekstrak tiap parameter-group menjadi ParamNode
 */
vector<ASTNode*> ASTConverter::convertFormalParameterList(ParseNode* node) {
    vector<ASTNode*> params;
    
    // children[0] = lparent
    // children[last] = rparent
    // [1]=parameter-group, (Berulang tiap 2 token: [2]=semicolon, [3]=parameter-group)
    for (size_t i = 1; i < node->getChildren().size() - 1; i += 2) {
        ParseNode* group = node->getChildren()[i];
        
        // <parameter-group> -> identifier-list + colon + (ident | array-type)
        vector<string> names = convertIdentifierList(group->getChildren()[0]);
        
        // group.children[1] = colon (diabaikan)
        // group.children[2] = ident | <array-type> (karena <type> tidak eksplisit dipanggil di param
        
        ASTNode* typeDef = nullptr;
        string typeStr = group->getChildren()[2]->getName();
        if (typeStr == "<array-type>") {
            typeDef = convertType(group->getChildren()[2]); 
        } else {
            // Jika hanya ident biasa (misal: "integer")
            string actualTypeName = group->getChildren()[2]->getToken().value;
            typeDef = new NamedTypeNode(actualTypeName);
        }
        
        // isVar diset false secara default, jika pass-by-reference maka true
        params.push_back(new ParamNode(names, typeDef, false));
    }
    
    return params;
}


// TIPE DATA
/*
 * SDT: <type> -> ident | <array-type> | <range> | <enumerated> | <record-type>
 * Semantic Action: Cek tipe anak, rutekan ke fungsi spesifik atau buat NamedTypeNode.
 */
ASTNode* ASTConverter::convertType(ParseNode* node) {
    ParseNode* actualType = node->getChildren()[0];
    string typeName = actualType->getName();
    if (typeName == "<array-type>") {
        return convertArrayType(actualType);
    } 
    else if (typeName == "<range>") {
        return convertRange(actualType);
    } 
    else if (typeName == "<enumerated>") {
        return convertEnumerated(actualType);
    } 
    else if (typeName == "<record-type>") {
        return convertRecordType(actualType);
    }
    else {
        string actualName = actualType->getToken().value;
        return new NamedTypeNode(actualName);
    }
}

/*
 * SDT: <array-type> -> arraysy + lbrack + (range | ident) + rbrack + ofsy + type
 * Semantic Action: return new ArrayTypeNode(indexType, elementType)
 */
ASTNode* ASTConverter::convertArrayType(ParseNode* node) {
    // children[0] = arraysy
    // children[1] = lbrack '['
    // children[2] = <range> ATAU ident
    // children[3] = rbrack ']'
    // children[4] = ofsy
    // children[5] = <type>
    
    ASTNode* indexType = nullptr;
    ParseNode* idxNode = node->getChildren()[2];
    
    if (idxNode->getName() == "<range>") {
        indexType = convertRange(idxNode);
    } else {
        // Jika index menggunakan nama tipe (misal: array [char] of integer)
        indexType = new NamedTypeNode(idxNode->getToken().value);
    }

    ASTNode* elementType = convertType(node->getChildren()[5]);
    
    return new ArrayTypeNode(indexType, elementType);
}

/*
 * SDT: <range> -> constant + period + period + constant
 * Semantic Action: return new RangeNode(lowerBound, upperBound)
 */
ASTNode* ASTConverter::convertRange(ParseNode* node) {
    // children[0] = <constant>
    // children[1] = period '.'
    // children[2] = period '.'
    // children[3] = <constant>
    
    ASTNode* lowerBound = convertConstant(node->getChildren()[0]);
    ASTNode* upperBound = convertConstant(node->getChildren()[3]);
    
    return new RangeNode(lowerBound, upperBound);
}

/*
 * SDT: <enumerated> -> lparent + ident + (comma + ident)* + rparent
 * Semantic Action: Kumpulkan ident ke vector string, return new EnumNode(identifiers)
 */
ASTNode* ASTConverter::convertEnumerated(ParseNode* node) {
    vector<string> identifiers;
    
    // children[0] = lparent '('
    // children[last] = rparent ')'
    // [1]=ident, (Pola berulang tiap 2 token: [2]=comma, [3]=ident)
    for (size_t i = 1; i < node->getChildren().size() - 1; i += 2) {
        identifiers.push_back(node->getChildren()[i]->getToken().value);
    }
    
    return new EnumNode(identifiers);
}

/*
 * SDT: <record-type> -> recordsy + <field-list> + endsy
 * <field-list> -> <field-part> + (semicolon + <field-part>)*
 * <field-part> -> <identifier-list> + colon + <type>
 * Semantic Action: Loop field-list, buat VarDeclNode untuk tiap field, return RecordTypeNode
 */
ASTNode* ASTConverter::convertRecordType(ParseNode* node) {
    vector<ASTNode*> fields;
    
    // children[0] = recordsy
    // children[1] = <field-list>
    // children[2] = endsy
    ParseNode* fieldList = node->getChildren()[1];

    // Pola: [0]=field-part, (Pola berulang tiap 2 token: [1]=semicolon, [2]=field-part)
    for (size_t i = 0; i < fieldList->getChildren().size(); i += 2) {
        ParseNode* fieldPart = fieldList->getChildren()[i];
        
        // <field-part> -> [0]=identifier-list, [1]=colon, [2]=<type>
        vector<string> names = convertIdentifierList(fieldPart->getChildren()[0]);
        ASTNode* typeDef = convertType(fieldPart->getChildren()[2]);

        fields.push_back(new VarDeclNode(names, typeDef));
    }
    
    return new RecordTypeNode(fields);
}

/*
 * SDT: <constant> -> charcon | string | [(plus | minus)? + (ident | intcon | realcon)]
 * Semantic Action: Ekstrak nilai string, buat NumberNode, StringNode, CharNode, atau VarNode
 */
ASTNode* ASTConverter::convertConstant(ParseNode* node) {
    size_t idx = 0;
    string sign = "";
    
    // Cek apakah ada tanda unary plus/minus
    TokenType firstTokenType = node->getChildren()[0]->getToken().type;
    if (firstTokenType == TokenType::PLUS || firstTokenType == TokenType::MINUS) {
        sign = node->getChildren()[0]->getToken().value;
        idx++;
    }

    ParseNode* valNode = node->getChildren()[idx];
    string valStr = sign + valNode->getToken().value; // Gabungkan, misal "-" dan "5" menjadi "-5"
    if (valStr.empty()) return nullptr;
 
    TokenType tType = valNode->getToken().type; // Ambil tipenya dari Lexer

    if (tType == TokenType::CHARCON) {
        return new CharNode(valStr);
    } 
    else if (tType == TokenType::STRING) {
        return new StringNode(valStr);
    } 
    else if (tType == TokenType::REALCON) {
        return new NumberNode(valStr, true); // true = isReal
    } 
    else if (tType == TokenType::INTCON) {
        return new NumberNode(valStr, false); // false = isInteger
    }
    // Fallback untuk Identifier murni
    return new VarNode(valStr);
}


// STATEMENT & COMPONENT VARIABLE
/*
 * SDT: <statement> -> (<assignment-statement> | <if-statement> | <case-statement> | 
 * <while-statement> | <repeat-statement> | <for-statement> | <procedure/function-call>)?
 * Semantic Action: Rutekan ke fungsi konversi spesifik.
 */
ASTNode* ASTConverter::convertStatement(ParseNode* node) {
    if (node->getChildren().empty()) return nullptr; // Empty statement (misal: 'begin ; end')

    ParseNode* actualStmt = node->getChildren()[0];
    string stmtType = actualStmt->getName();

    if (stmtType == "<assignment-statement>") return convertAssignment(actualStmt);
    if (stmtType == "<if-statement>") return convertIf(actualStmt);
    if (stmtType == "<case-statement>") return convertCase(actualStmt);
    if (stmtType == "<while-statement>") return convertWhile(actualStmt);
    if (stmtType == "<repeat-statement>") return convertRepeat(actualStmt);
    if (stmtType == "<for-statement>") return convertFor(actualStmt);
    if (stmtType == "<procedure/function-call>") return convertProcCall(actualStmt);

    return nullptr;
}

/*
 * SDT: <assignment-statement> -> <variable> + becomes + <expression>
 * Semantic Action: return new AssignNode(target.node, value.node)
 */
ASTNode* ASTConverter::convertAssignment(ParseNode* node) {
    // children[0] = <variable>
    // children[1] = becomes (:=) (diabaikan)
    // children[2] = <expression>
    
    ASTNode* target = convertVariable(node->getChildren()[0]);
    ASTNode* value = convertExpression(node->getChildren()[2]);
    
    return new AssignNode(target, value);
}

/*
 * SDT: <if-statement> -> ifsy + <expression> + thensy + <statement> + (elsy + <statement>)?
 * Semantic Action: return new IfNode(condition, thenBlock, elseBlock)
 */
ASTNode* ASTConverter::convertIf(ParseNode* node) {
    // children[0] = ifsy
    // children[1] = <expression> (Kondisi)
    // children[2] = thensy
    // children[3] = <statement> (Blok Benar)
    
    ASTNode* condition = convertExpression(node->getChildren()[1]);
    ASTNode* thenBlock = convertStatement(node->getChildren()[3]);
    ASTNode* elseBlock = nullptr;
    
    // Pengecekan branch else
    if (node->getChildren().size() > 4) {
        // children[4] = elsy
        // children[5] = <statement> (Blok Salah)
        elseBlock = convertStatement(node->getChildren()[5]);
    }
    
    return new IfNode(condition, thenBlock, elseBlock);
}

/*
 * SDT: <case-statement> -> casesy + <expression> + ofsy + <case-block> + endsy
 * Semantic Action: Evaluasi kondisi, lalu ratakan (flatten) semua <case-block> bersarang menjadi vector.
 */
ASTNode* ASTConverter::convertCase(ParseNode* node) {
    // children[0] = casesy
    // children[1] = <expression> (Variabel/kondisi yang dicek)
    // children[2] = ofsy
    // children[3] = <case-block> (Blok case pertama)
    // children[last] = endsy
    
    ASTNode* condition = convertExpression(node->getChildren()[1]);
    vector<ASTNode*> caseBlocks;
    
    // Mulai dari blok case yang paling luar
    ParseNode* currentBlock = node->getChildren()[3];
    
    // Looping untuk menelusuri sarang (linked-list traversal)
    while (currentBlock != nullptr) {
        // Konversi blok saat ini dan tambahkan
        caseBlocks.push_back(convertCaseBlock(currentBlock));
        
        // Cari apakah ada lanjutan
        ParseNode* nextBlock = nullptr;
        for (ParseNode* child : currentBlock->getChildren()) {
            if (child->getName() == "<case-block>") {
                nextBlock = child;
                break;
            }
        }
        
        // Masuk ke berikutnya
        currentBlock = nextBlock; 
    }
    
    return new CaseNode(condition, caseBlocks);
}

/*
 * SDT: <case-block> -> <constant> + (comma + <constant>)* + colon + <statement> + (semicolon + <case-block>?)*
 * Semantic Action: Ekstrak semua konstanta pembanding, dan 1 statement eksekusi.
 */
ASTNode* ASTConverter::convertCaseBlock(ParseNode* node) {
    vector<ASTNode*> constants;
    ASTNode* statement = nullptr;
    
    // Iterasi semua anak di level <case-block> saat ini
    for (ParseNode* child : node->getChildren()) {
        string childName = child->getName();
        
        if (childName == "<constant>") {
            constants.push_back(convertConstant(child));
        } 
        else if (childName == "<statement>") {
            statement = convertStatement(child);
        }
        // Jika ada token semicolon atau "<case-block>" bersarang, abaikan karena sudah diurus while loop.
    }
    
    return new CaseBlockNode(constants, statement);
}

/*
 * SDT: <while-statement> -> whilesy + <expression> + dosy + <compound-statement> + semicolon
 * Semantic Action: return new WhileNode(condition, loopBlock)
 */
ASTNode* ASTConverter::convertWhile(ParseNode* node) {
    // children[0] = whilesy
    // children[1] = <expression>
    // children[2] = dosy
    // children[3] = <compound-statement>
    // children[4] = semicolon (diabaikan)
    
    ASTNode* condition = convertExpression(node->getChildren()[1]);
    ASTNode* loopBlock = convertCompoundStatement(node->getChildren()[3]);
    
    return new WhileNode(condition, loopBlock);
}

/*
 * SDT: <repeat-statement> -> repeatsy + <statement-list> + untilsy + <expression>
 * Semantic Action: return new RepeatNode(statements, condition)
 */
ASTNode* ASTConverter::convertRepeat(ParseNode* node) {
    // children[0] = repeatsy
    // children[1] = <statement-list>
    // children[2] = untilsy
    // children[3] = <expression>
    
    vector<ASTNode*> stmts = convertStatementList(node->getChildren()[1]);
    ASTNode* condition = convertExpression(node->getChildren()[3]);
    
    return new RepeatNode(stmts, condition);
}

/*
 * SDT: <for-statement> -> forsy + ident + becomes + <expression> + (tosy | downtosy) + 
 * <expression> + dosy + <compound-statement> + semicolon
 * Semantic Action: return new ForNode(iterator, start, end, isDownto, block)
 */
ASTNode* ASTConverter::convertFor(ParseNode* node) {
    // children[0] = forsy
    // children[1] = ident (iterator)
    // children[2] = becomes (:=)
    // children[3] = <expression> (nilai awal)
    // children[4] = tosy ATAU downtosy
    // children[5] = <expression> (nilai akhir)
    // children[6] = dosy
    // children[7] = <compound-statement>
    // children[8] = semicolon
    
    string iteratorName = node->getChildren()[1]->getToken().value;
    ASTNode* startVal = convertExpression(node->getChildren()[3]);
    
    bool isDownto = (node->getChildren()[4]->getToken().type == TokenType::DOWNTO);
    
    ASTNode* endVal = convertExpression(node->getChildren()[5]);
    ASTNode* loopBlock = convertCompoundStatement(node->getChildren()[7]);
    
    return new ForNode(iteratorName, startVal, endVal, isDownto, loopBlock);
}

/*
 * SDT: <procedure/function-call> -> ident + (lparent + <parameter-list>? + rparent)?
 * Semantic Action: return new ProcCallNode(procName, arguments)
 */
ASTNode* ASTConverter::convertProcCall(ParseNode* node) {
    // children[0] = ident (nama prosedur)
    string procName = node->getChildren()[0]->getToken().value;
    vector<ASTNode*> args;
    
    // Pengecekan keberadaan parameter
    if (node->getChildren().size() > 1) {
        // children[1] = lparent '('
        // Jika index ke-2 adalah parameter-list (bukan rparent seperti '()')
        if (node->getChildren()[2]->getName() == "<parameter-list>") {
            args = convertParameterList(node->getChildren()[2]);
        }
    }
    
    return new ProcCallNode(procName, args);
}

/*
 * SDT: <parameter-list> -> <expression> + (comma + <expression>)*
 * Semantic Action: Kumpulkan argumen ke vector
 */
vector<ASTNode*> ASTConverter::convertParameterList(ParseNode* node) {
    vector<ASTNode*> args;
    // [0]=expr (Pola berulang tiap 2 token: [1]=comma, [2]=expr)
    for (size_t i = 0; i < node->getChildren().size(); i += 2) {
        args.push_back(convertExpression(node->getChildren()[i]));
    }
    return args;
}

/*
 * SDT: <variable> -> ident + (<component-variable>)*
 * <component-variable> -> (lbrack + <index-list> + rbrack) | (period + ident)
 * Semantic Action: Bungkus node variabel dasar dengan ArrayAccessNode atau RecordAccessNode
 */
ASTNode* ASTConverter::convertVariable(ParseNode* node) {
    if (!node || node->getChildren().empty()) return nullptr;

    // Dasar variabel: children[0] ident
    string baseName = node->getChildren()[0]->getToken().value;
    ASTNode* currentVar = new VarNode(baseName);
    
    // Looping jika ada akses ke dalam array atau record
    for (size_t i = 1; i < node->getChildren().size(); i++) {
        ParseNode* comp = node->getChildren()[i]; // <component-variable>
        TokenType firstCompType = comp->getChildren()[0]->getToken().type;
        
        if (firstCompType == TokenType::LBRACK) {
            // comp->children[1] adalah <index-list>
            vector<ASTNode*> indices = convertIndexList(comp->getChildren()[1]);
            currentVar = new ArrayAccessNode(currentVar, indices);
        } 
        else if (firstCompType == TokenType::PERIOD) { // Akses Record
            // comp->children[1] adalah ident (nama field)
            string fieldName = comp->getChildren()[1]->getToken().value;
            currentVar = new RecordAccessNode(currentVar, fieldName);
        }
    }
    
    return currentVar;
}

/*
 * SDT: <index-list> -> (intcon | charcon | ident) + (comma + <index-list>)*
 * Catatan: Karena di ASTNode.hpp ArrayAccessNode menerima vector<ASTNode*>,
 * kita treat isi dari index-list sebagai Expression atau Constant. 
 */
vector<ASTNode*> ASTConverter::convertIndexList(ParseNode* node) {
    vector<ASTNode*> indices;
    // lompat 2 untuk melewati koma
    for (size_t i = 0; i < node->getChildren().size(); i += 2) {
        ParseNode* valNode = node->getChildren()[i];
        
        string valStr = valNode->getToken().value;
        if (valStr.empty()) continue;
        if (isalpha(valStr[0])) {
            indices.push_back(new VarNode(valStr));
        } else if (valStr.length() >= 2 && valStr.front() == '\'' && valStr.back() == '\'') {
            indices.push_back(new CharNode(valStr));
        } else {
            indices.push_back(new NumberNode(valStr, false));
        }
    }
    return indices;
}


// EXPRESSION & FACTOR
/*
 * SDT: <expression> -> <simple-expression> + (<relational-operator> + <simple-expression>)?
 * Semantic Action: Jika ada relasi, return BinOpNode. Jika tidak, passthrough (teruskan simple-expr).
 */
ASTNode* ASTConverter::convertExpression(ParseNode* node) {
    // children[0] = <simple-expression>
    ASTNode* left = convertSimpleExpression(node->getChildren()[0]);

    // Cek apakah ada relational operator
    if (node->getChildren().size() > 1) {
        // children[1] = <relational-operator> (isinya eql, neq, lss, gtr, dsb)
        // children[2] = <simple-expression>
        
        string op = node->getChildren()[1]->getChildren()[0]->getToken().value;
        ASTNode* right = convertSimpleExpression(node->getChildren()[2]);
        
        return new BinOpNode(op, left, right);
    }

    return left;
}

/*
 * SDT: <simple-expression> -> (plus | minus)? + <term> + (<additive-operator> + <term>)*
 * Semantic Action: Tangani unary operator (jika ada). Loop untuk additive ops, gabung jadi BinOpNode.
 */
ASTNode* ASTConverter::convertSimpleExpression(ParseNode* node) {
    size_t idx = 0;
    ASTNode* currentLeft = nullptr;

    // Cek unary operator di awal
    TokenType firstType = node->getChildren()[0]->getToken().type;
    if (firstType == TokenType::PLUS || firstType == TokenType::MINUS) {
        string sign = node->getChildren()[0]->getToken().value;
        idx++;
        ASTNode* termNode = convertTerm(node->getChildren()[idx]);
        currentLeft = new UnaryOpNode(sign, termNode);
    } else {
        currentLeft = convertTerm(node->getChildren()[idx]);
    }

    idx++; // Pindah ke kemungkinan <additive-operator>

    // Loop untuk (additive-operator + term)*
    // Pola berulang tiap 2 token: [op], [term]
    while (idx < node->getChildren().size()) {
        string op = node->getChildren()[idx]->getChildren()[0]->getToken().value; // misal "+" atau "or"
        idx++;
        
        ASTNode* nextRight = convertTerm(node->getChildren()[idx]);
        idx++;
        
        // Gabungkan
        currentLeft = new BinOpNode(op, currentLeft, nextRight);
    }

    return currentLeft;
}

/*
 * SDT: <term> -> <factor> + (<multiplicative-operator> + <factor>)*
 * Semantic Action: Loop untuk multiplicative ops, gabung ke kiri jadi BinOpNode.
 */
ASTNode* ASTConverter::convertTerm(ParseNode* node) {
    // children[0] = <factor>
    ASTNode* currentLeft = convertFactor(node->getChildren()[0]);
    
    size_t idx = 1;
    
    // Loop untuk (multiplicative-operator + factor)*
    while (idx < node->getChildren().size()) {
        string op = node->getChildren()[idx]->getChildren()[0]->getToken().value; // misal "*" atau "and"
        idx++;
        
        ASTNode* nextRight = convertFactor(node->getChildren()[idx]);
        idx++;
        
        currentLeft = new BinOpNode(op, currentLeft, nextRight);
    }
    
    return currentLeft;
}

/*
 * SDT: <factor> -> ident | intcon | realcon | charcon | string | 
 * (lparent + <expression> + rparent) | (notsy + <factor>) | <procedure/function-call> | <variable>
 * Semantic Action: Cek isi node, rutekan/bungkus ke kelas turunan yang sesuai.
 */
ASTNode* ASTConverter::convertFactor(ParseNode* node) {
    ParseNode* child = node->getChildren()[0];
    
    // Pengecekan non-terminal
    string nodeName = child->getName();

    // Jika berupa pemanggilan fungsi/prosedur
    if (nodeName == "<procedure/function-call>") {
        return convertProcCall(child);
    }
    
    // Jika berupa variabel (termasuk akses array/record)
    if (nodeName == "<variable>") {
        return convertVariable(child);
    }

    // Pengecekan terminal
    string valStr = child->getToken().value;

    if (valStr.empty()) return nullptr;

    // Jika berupa kurung (lparent + <expression> + rparent)
    if (valStr == "(") {
        // children[1] adalah <expression>
        return convertExpression(node->getChildren()[1]); // kurung dibuang
    }
    
    // Jika berupa NOT (notsy + <factor>)
    if (valStr == "not" || valStr == "NOT") {
        return new UnaryOpNode("not", convertFactor(node->getChildren()[1]));
    }
    
    TokenType tType = child->getToken().type;
    if (tType == TokenType::CHARCON) {
        return new CharNode(valStr);
    } 
    else if (tType == TokenType::STRING) {
        return new StringNode(valStr);
    } 
    else if (tType == TokenType::REALCON) {
        return new NumberNode(valStr, true); // true = isReal
    } 
    else if (tType == TokenType::INTCON) {
        return new NumberNode(valStr, false); // false = isInteger
    }

    return new VarNode(valStr);
}