#pragma once
#include <vector>
#include <string>
#include "Token.hpp"

class Lexer {
private :
    enum class State
    {
        START,
        IDENT,
        INT,
        REAL,
        STRING,
        QUOTE_END,
        MINUS,
        UNKNOWN,

        COLON,
        LT, // smaller
        GT, // greater
        EQ, // equal
        CHAR,
        NEXT_CHAR,

        A,
        AN,
        AND,
        AR,
        ARR,
        ARRA,
        ARRAY,

        B,
        BE,
        BEG,
        BEGI,
        BEGIN,

        C,
        CA,
        CAS,
        CASE,
        CO,
        CON,
        CONS,
        CONST,

        D,
        DI,
        DIV,
        DO,
        DOW,
        DOWN,
        DOWNT,
        DOWNTO,

        E,
        EL,
        ELS,
        ELSE,
        EN,
        END,

        F,
        FO,
        FOR,
        FUN,
        FUNC,
        FUNCT,
        FUNCTI,
        FUNCTIO,
        FUNCTION,

        I,
        IF,

        M,
        MO,
        MOD,

        N,
        NO,
        NOT,

        O,
        OF,
        OR,

        P,
        PR,
        PRO,
        PROG,
        PROGR,
        PROGRA,
        PROGRAM,
        PROC,
        PROCE,
        PROCED,
        PROCEDU,
        PROCEDUR,
        PROCEDURE,

        R,
        RE,
        REC,
        RECO,
        RECOR,
        RECORD,
        REP,
        REPE,
        REPEA,
        REPEAT,

        T,
        TH,
        THE,
        THEN,
        TO,
        TY,
        TYP,
        TYPE,

        U,
        UN,
        UNT,
        UNTI,
        UNTIL,

        V,
        VA,
        VAR,

        W,
        WH,
        WHI,
        WHIL,
        WHILE,
            
        
    };

    enum class Skip{
        IGNORE,
        BRACE_COMMENT_LEFT,
        COMMENT_OPEN,
        COMMENT_BODY,
        COMMENT_STAR
    };
    

    string line; 

    int pos;

    char current();
    char adv ();
    bool isEnd();

    void skip();

    bool isAlphabet(char c);
    bool isDigit(char c);
    bool isAlphanumeric(char c);
    bool isSpace(char c);
    bool isSymbol(char c);

    char toLowerChar(char c);
    Token makeToken(TokenType type, int start, int end);
    
    Token nextToken();

public: 
    Lexer(const string& input);

    vector<Token> tokenize();
};