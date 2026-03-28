#include "Token.hpp"

string typeToString(TokenType type){
    switch (type){
        case INTCON : 
            return "intcon";
        case REALCON :
            return "realcon";
        case CHARCON :
            return "charcon";
        case STRING :
            return "string";
        case IDENTIFIER :
            return "ident";
        case PLUS :
            return "plus";
        case MINUS :
            return "minus";
        case TIMES :
            return "times";
        case IDIV :
            return "idiv";
        case RDIV :
            return "rdiv";
        case MOD :
            return "imod";
        case AND :
            return "andsy";
        case OR :
            return "orsy";
        case NOT :
            return "notsy";
        case EQL :
            return "eql";
        case NEQ :
            return "neq";
        case GTR : 
            return "gtr";
        case GEQ :
            return "geq";
        case LSS :
            return "lss";
        case LEQ :
            return "leq";
        case ASSIGN :
            return "becomes";
        case LPARENT :
            return "lparent";
        case RPARENT :
            return "rparent";
        case LBRACK :
            return "lbrack";
        case RBRACK :
            return "rbrack";
        case COMMA :
            return "comma";
        case SEMICOLON :
            return "semicolon";
        case PERIOD :
            return "period";
        case COLON :
            return "colon";
        case CONST :
            return "constsy";
        case TYPE :
            return "typesy";
        case VAR :
            return "varsy";
        case FUNCTION :
            return "functionsy";
        case PROCEDURE :
            return "proceduresy";
        case ARRAY :
            return "arraysy";
        case RECORD :
            return "recordsy";
        case PROGRAM :
            return "programsy";
        case BEGIN :
            return "beginsy";
        case END :
            return "endsy";
        case IF :
            return "ifsy";
        case THEN :
            return "thensy";
        case ELSE :
            return "elsesy";
        case CASE :
            return "casesy";
        case OF :
            return "ofsy";
        case REPEAT :
            return "repeatsy";
        case UNTIL :
            return "untilsy";
        case WHILE :
            return "whilesy";
        case DO :
            return "dosy";
        case FOR :
            return "forsy";
        case TO :
            return "tosy";
        case DOWNTO :
            return "downtosy";
        case COMMENT :
            return "comment";
    }
    return "unknown";
}