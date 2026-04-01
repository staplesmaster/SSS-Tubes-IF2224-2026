#pragma once

#include <cstddef>
#include <string>
using namespace std;

enum TokenType{
    INTCON, // int (1, 3, 48)  
    REALCON, // riil (3.14, 26.7)
    CHARCON, // char harus '..' dengan satu karakter ('a', 'b', '1')
    STRING, // sekuens karakter '...' dengan lebih dari satu karakter ('IRK', 'TBFO')
    IDENTIFIER, 

    // OPERATOR ARITMATIKA 
    PLUS, // + 
    MINUS, // -
    TIMES, // *
    IDIV, // div 
    RDIV, // pembagian menggunakan '/'
    MOD, // MOD

    // OPERATOR LOGIKA
    AND, // AND
    OR, // OR
    NOT, // NOT 

    // COMPARE
    EQL, // == (equal)
    NEQ, // <> (not equal)
    GTR, // > (greater than)
    GEQ, // >= (greater than or equal)
    LSS, // < (less than)
    LEQ, // <= (less than or equal)

    // ASSIGNMENT SAMA SYMBOL
    ASSIGN, // := , tapi nama tokennya becomes
    LPARENT, // (
    RPARENT, // )
    LBRACK, // [
    RBRACK, // ]
    COMMA, // ,
    SEMICOLON, // ;
    PERIOD, // .
    COLON, // :

    // DEKLARASI
    CONST, // const 
    TYPE, // type
    VAR, // var
    FUNCTION, // function
    PROCEDURE, // procedure
    ARRAY, // array
    RECORD, // record
    PROGRAM, // program

    // DAN LAIN LAIN
    BEGIN, // begin
    END, // end
    IF, // if
    THEN, // then
    ELSE, // else
    CASE, // case
    OF, // of
    REPEAT, // repeat
    UNTIL, // until
    WHILE, // while
    DO, // do 
    FOR, // for 
    TO, // tp 
    DOWNTO, // downto
    COMMENT, // { } atau (*  *)
    UNKNOWN
};

struct Token {
    TokenType type;
    string value;
    size_t start;
    size_t end;
};

string typeToString(TokenType type);