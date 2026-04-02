#include "Lexer.hpp"

#include <stdexcept>

Lexer::Lexer(const string& input) : line(input), pos(0) {}

char Lexer::current() {
    if (isEnd()) {
        return '\0';
    }
    return line[pos];
}

char Lexer::adv() {
    if (isEnd()) {
        return '\0';
    }
    return line[pos++];
}

bool Lexer::isEnd() {
    return pos >= (int)line.size();
}

bool Lexer::isSymbol(char c) {
    switch (c) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '<':
        case '>':
        case ':':
        case '(':
        case ')':
        case '[':
        case ']':
        case ';':
        case ',':
        case '.':
        case ' ':
            return true;
        default:
            return false;
    }
}

bool Lexer::isAlphabet(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlphanumeric(char c) {
    return isAlphabet(c) || isDigit(c);
}

bool Lexer::isSpace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

char Lexer::toLowerChar(char c) {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c + 32);
    }
    return c;
}

Token Lexer::makeToken(TokenType type, int start, int end) {
    return {type, line.substr(start, end - start), start, end};
}

void Lexer::skip() {
    

    Skip skipState = Skip::IGNORE;
    int parenCommentStart = 0;

    while (true) {
        char c = current();

        switch (skipState) {
            case Skip::IGNORE:
                if (isEnd()) {
                    return;
                }
                if (isSpace(c)) {
                    adv();
                } else if (c == '{') {
                    adv();
                    skipState = Skip::BRACE_COMMENT_LEFT;
                } else if (c == '(') {
                    parenCommentStart = pos; // posisi awal comment
                    adv();
                    skipState = Skip::COMMENT_OPEN;
                } else {
                    return;
                }
                break;

            case Skip::BRACE_COMMENT_LEFT:
                if (isEnd()) {
                    throw runtime_error("Unclosed comment");
                }
                if (c == '}') {
                    adv();
                    skipState = Skip::IGNORE;
                } else {
                    adv();
                }
                break;

            case Skip::COMMENT_OPEN:
                if (isEnd()) {
                    pos = parenCommentStart;
                    return;
                }
                if (c == '*') {
                    adv();
                    skipState = Skip::COMMENT_BODY; // memulai comment
                } else {
                    pos = parenCommentStart;
                    return;
                }
                break;

            case Skip::COMMENT_BODY:
                if (isEnd()) {
                    throw runtime_error("Unclosed comment"); // /* tidak ada penutup 
                }
                if (c == '*') {
                    adv();
                    skipState = Skip::COMMENT_STAR;
                } else {
                    adv(); // skip 
                }
                break;

            case Skip::COMMENT_STAR:
                if (isEnd()) {
                    throw runtime_error("Unclosed comment");
                }
                if (c == ')') {
                    adv();
                    skipState = Skip::IGNORE;
                } else if (c == '*') {
                    adv();
                } else {
                    adv();
                    skipState = Skip::COMMENT_BODY;
                }
                break;
        }
    }
}

vector<Token> Lexer::tokenize() {
    vector<Token> tokens;
    while (!isEnd()) {
        skip();
        if (isEnd()) {
            break;
        }
        tokens.push_back(nextToken());
    }
    return tokens;
}

Token Lexer::nextToken() {
    State state = State::START;
    int tokenStart = pos;
    bool hasFraction = false;

    while (true) {
        char c = current();
        char lower = toLowerChar(c);

        switch (state) {
            case State::START:
                if (isAlphabet(c)) {
                    tokenStart = pos;
                    char first = toLowerChar(adv());
                    switch (first) {
                        case 'a': state = State::A; break;
                        case 'b': state = State::B; break;
                        case 'c': state = State::C; break;
                        case 'd': state = State::D; break;
                        case 'e': state = State::E; break;
                        case 'f': state = State::F; break;
                        case 'i': state = State::I; break;
                        case 'm': state = State::M; break;
                        case 'n': state = State::N; break;
                        case 'o': state = State::O; break;
                        case 'p': state = State::P; break;
                        case 'r': state = State::R; break;
                        case 't': state = State::T; break;
                        case 'u': state = State::U; break;
                        case 'v': state = State::V; break;
                        case 'w': state = State::W; break;
                        default: state = State::IDENT; break;
                    }
                } else if (isDigit(c)) {
                    tokenStart = pos;
                    adv();
                    state = State::INT;
                } else if (c == '\'') {
                    tokenStart = pos;
                    adv();
                    state = State::CHAR;
                } else if (c == ':') {
                    tokenStart = pos;
                    adv();
                    state = State::COLON;
                } else if (c == '<') {
                    tokenStart = pos;
                    adv();
                    state = State::LT;
                } else if (c == '>') {
                    tokenStart = pos;
                    adv();
                    state = State::GT;
                } else if (c == '+') {
                    tokenStart = pos;
                    adv();
                    return makeToken(PLUS, tokenStart, pos);
                } else if (c == '-') {
                    tokenStart = pos;
                    adv();
                    state = State::MINUS;
                } else if (c == '*') {
                    tokenStart = pos;
                    adv();
                    return makeToken(TIMES, tokenStart, pos);
                } else if (c == '/') {
                    tokenStart = pos;
                    adv();
                    return makeToken(RDIV, tokenStart, pos);
                } else if (c == '(') {
                    tokenStart = pos;
                    adv();
                    return makeToken(LPARENT, tokenStart, pos);
                } else if (c == ')') {
                    tokenStart = pos;
                    adv();
                    return makeToken(RPARENT, tokenStart, pos);
                } else if (c == '[') {
                    tokenStart = pos;
                    adv();
                    return makeToken(LBRACK, tokenStart, pos);
                } else if (c == ']') {
                    tokenStart = pos;
                    adv();
                    return makeToken(RBRACK, tokenStart, pos);
                } else if (c == ';') {
                    tokenStart = pos;
                    adv();
                    return makeToken(SEMICOLON, tokenStart, pos);
                } else if (c == ',') {
                    tokenStart = pos;
                    adv();
                    return makeToken(COMMA, tokenStart, pos);
                } else if (c == '.') {
                    tokenStart = pos;
                    adv();
                    return makeToken(PERIOD, tokenStart, pos);
                } else {
                    tokenStart = pos;
                    adv();
                    state = State::UNKNOWN;
                }
                break;
            
            case State::UNKNOWN:
                if (isAlphanumeric(c) || !isSymbol(c)) {
                    adv();
                }
                else{
                    return makeToken(UNKNOWN, tokenStart, pos);
                }
                break;
            case State::MINUS:
                if (isDigit(c)){
                    state = State::INT;
                    adv();
                }else{
                    return makeToken(MINUS, tokenStart,pos);
                }
                break;

            case State::CHAR:
                if (isAlphanumeric(c)){
                    adv();
                    state = State::NEXT_CHAR;
                }else if (c == '\''){
                    adv();
                    return makeToken(STRING, tokenStart, pos);
                }
                else{
                    return makeToken(UNKNOWN, tokenStart, pos);
                }
                break;
            case State::NEXT_CHAR:
                if(isAlphanumeric(c)){
                    adv();
                    state = State::STRING;
                }else if (c == '\''){
                    adv();
                    return makeToken(CHARCON,tokenStart,pos);
                }else{
                    return makeToken(STRING, tokenStart,pos);
                }
                break;

            case State::IDENT:
                while (!isEnd() && isAlphanumeric(current())) {
                    adv();
                }
                return makeToken(IDENTIFIER, tokenStart, pos);

            case State::INT:
                if (isDigit(c)) {
                    adv();
                }else if (c == '.') {
                    adv();
                    state = State::REAL;
                    hasFraction = false;
                }else if(isAlphabet(c)){
                    adv();
                    state = State::UNKNOWN;
                }
                else {
                    return makeToken(INTCON, tokenStart, pos);
                }
                break;

            case State::REAL:
                if (isDigit(c)) {
                    adv();
                    hasFraction = true;
                } else {
                    if (!hasFraction) {
                        throw runtime_error("Invalid real number '" + line.substr(tokenStart, pos - tokenStart) + "'");
                    }
                    return makeToken(REALCON, tokenStart, pos);
                }
                break;

            case State::STRING:
                if (isEnd() || c == '\n' || c == '\r' || c == ';') {
                    return makeToken(UNKNOWN, tokenStart, pos);
                }
                if (c == '\'') {
                    adv();
                    state = State::QUOTE_END;
                } else {
                    adv();
                }
                break;
            
            case State::QUOTE_END:
                return makeToken(STRING, tokenStart, pos);
                break;
        

            case State::COLON:
                if (c == '=') {
                    adv();
                    return makeToken(ASSIGN, tokenStart, pos);
                }
                return makeToken(COLON, tokenStart, pos);

            case State::LT:
                if (c == '=') {
                    adv();
                    return makeToken(LEQ, tokenStart, pos);
                }
                if (c == '>') {
                    adv();
                    return makeToken(NEQ, tokenStart, pos);
                }
                return makeToken(LSS, tokenStart, pos);

            case State::GT:
                if (c == '=') {
                    adv();
                    return makeToken(GEQ, tokenStart, pos);
                }
                return makeToken(GTR, tokenStart, pos);

            case State::EQ:
                if (c == '=') {
                    adv();
                    return makeToken(EQL, tokenStart, pos);
                }else{
                    adv();
                    state = State::UNKNOWN;
                }
                break;
            case State::A:
                if (isAlphanumeric(c)) {
                    if (lower == 'n') {
                        adv();
                        state = State::AN;
                    } else if (lower == 'r') {
                        adv();
                        state = State::AR;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::AN:
                if (isAlphanumeric(c)) {
                    if (lower == 'd') {
                        adv();
                        state = State::AND;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::AND:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(AND, tokenStart, pos);
                }
                break;

            case State::AR:
                if (isAlphanumeric(c)) {
                    if (lower == 'r') {
                        adv();
                        state = State::ARR;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::ARR:
                if (isAlphanumeric(c)) {
                    if (lower == 'a') {
                        adv();
                        state = State::ARRA;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::ARRA:
                if (isAlphanumeric(c)) {
                    if (lower == 'y') {
                        adv();
                        state = State::ARRAY;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::ARRAY:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(ARRAY, tokenStart, pos);
                }
                break;

            case State::B:
                if (isAlphanumeric(c)) {
                    if (lower == 'e') {
                        adv();
                        state = State::BE;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::BE:
                if (isAlphanumeric(c)) {
                    if (lower == 'g') {
                        adv();
                        state = State::BEG;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::BEG:
                if (isAlphanumeric(c)) {
                    if (lower == 'i') {
                        adv();
                        state = State::BEGI;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::BEGI:
                if (isAlphanumeric(c)) {
                    if (lower == 'n') {
                        adv();
                        state = State::BEGIN;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::BEGIN:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(BEGIN, tokenStart, pos);
                }
                break;

            case State::C:
                if (isAlphanumeric(c)) {
                    if (lower == 'a') {
                        adv();
                        state = State::CA;
                    } else if (lower == 'o') {
                        adv();
                        state = State::CO;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::CA:
                if (isAlphanumeric(c)) {
                    if (lower == 's') {
                        adv();
                        state = State::CAS;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::CAS:
                if (isAlphanumeric(c)) {
                    if (lower == 'e') {
                        adv();
                        state = State::CASE;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::CASE:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(CASE, tokenStart, pos);
                }
                break;

            case State::CO:
                if (isAlphanumeric(c)) {
                    if (lower == 'n') {
                        adv();
                        state = State::CON;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::CON:
                if (isAlphanumeric(c)) {
                    if (lower == 's') {
                        adv();
                        state = State::CONS;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::CONS:
                if (isAlphanumeric(c)) {
                    if (lower == 't') {
                        adv();
                        state = State::CONST;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::CONST:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(CONST, tokenStart, pos);
                }
                break;

            case State::D:
                if (isAlphanumeric(c)) {
                    if (lower == 'i') {
                        adv();
                        state = State::DI;
                    } else if (lower == 'o') {
                        adv();
                        state = State::DO;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::DI:
                if (isAlphanumeric(c)) {
                    if (lower == 'v') {
                        adv();
                        state = State::DIV;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::DIV:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(IDIV, tokenStart, pos);
                }
                break;

            case State::DO:
                if (isAlphanumeric(c)) {
                    if (lower == 'w') {
                        adv();
                        state = State::DOW;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(DO, tokenStart, pos);
                }
                break;

            case State::DOW:
                if (isAlphanumeric(c)) {
                    if (lower == 'n') {
                        adv();
                        state = State::DOWN;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::DOWN:
                if (isAlphanumeric(c)) {
                    if (lower == 't') {
                        adv();
                        state = State::DOWNT;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::DOWNT:
                if (isAlphanumeric(c)) {
                    if (lower == 'o') {
                        adv();
                        state = State::DOWNTO;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::DOWNTO:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(DOWNTO, tokenStart, pos);
                }
                break;

            case State::E:
                if (isAlphanumeric(c)) {
                    if (lower == 'l') {
                        adv();
                        state = State::EL;
                    } else if (lower == 'n') {
                        adv();
                        state = State::EN;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::EL:
                if (isAlphanumeric(c)) {
                    if (lower == 's') {
                        adv();
                        state = State::ELS;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::ELS:
                if (isAlphanumeric(c)) {
                    if (lower == 'e') {
                        adv();
                        state = State::ELSE;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::ELSE:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(ELSE, tokenStart, pos);
                }
                break;

            case State::EN:
                if (isAlphanumeric(c)) {
                    if (lower == 'd') {
                        adv();
                        state = State::END;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::END:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(END, tokenStart, pos);
                }
                break;

            case State::F:
                if (isAlphanumeric(c)) {
                    if (lower == 'o') {
                        adv();
                        state = State::FO;
                    } else if (lower == 'u') {
                        adv();
                        state = State::FUN;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::FO:
                if (isAlphanumeric(c)) {
                    if (lower == 'r') {
                        adv();
                        state = State::FOR;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::FOR:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(FOR, tokenStart, pos);
                }
                break;

            case State::FUN:
                if (isAlphanumeric(c)) {
                    if (lower == 'c') {
                        adv();
                        state = State::FUNC;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::FUNC:
                if (isAlphanumeric(c)) {
                    if (lower == 't') {
                        adv();
                        state = State::FUNCT;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::FUNCT:
                if (isAlphanumeric(c)) {
                    if (lower == 'i') {
                        adv();
                        state = State::FUNCTI;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::FUNCTI:
                if (isAlphanumeric(c)) {
                    if (lower == 'o') {
                        adv();
                        state = State::FUNCTIO;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::FUNCTIO:
                if (isAlphanumeric(c)) {
                    if (lower == 'n') {
                        adv();
                        state = State::FUNCTION;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::FUNCTION:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(FUNCTION, tokenStart, pos);
                }
                break;

            case State::I:
                if (isAlphanumeric(c)) {
                    if (lower == 'f') {
                        adv();
                        state = State::IF;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::IF:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(IF, tokenStart, pos);
                }
                break;

            case State::M:
                if (isAlphanumeric(c)) {
                    if (lower == 'o') {
                        adv();
                        state = State::MO;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::MO:
                if (isAlphanumeric(c)) {
                    if (lower == 'd') {
                        adv();
                        state = State::MOD;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::MOD:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(MOD, tokenStart, pos);
                }
                break;

            case State::N:
                if (isAlphanumeric(c)) {
                    if (lower == 'o') {
                        adv();
                        state = State::NO;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::NO:
                if (isAlphanumeric(c)) {
                    if (lower == 't') {
                        adv();
                        state = State::NOT;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::NOT:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(NOT, tokenStart, pos);
                }
                break;

            case State::O:
                if (isAlphanumeric(c)) {
                    if (lower == 'f') {
                        adv();
                        state = State::OF;
                    } else if (lower == 'r') {
                        adv();
                        state = State::OR;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::OF:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(OF, tokenStart, pos);
                }
                break;

            case State::OR:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(OR, tokenStart, pos);
                }
                break;

            case State::P:
                if (isAlphanumeric(c)) {
                    if (lower == 'r') {
                        adv();
                        state = State::PR;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::PR:
                if (isAlphanumeric(c)) {
                    if (lower == 'o') {
                        adv();
                        state = State::PRO;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::PRO:
                if (isAlphanumeric(c)) {
                    if (lower == 'g') {
                        adv();
                        state = State::PROG;
                    } else if (lower == 'c') {
                        adv();
                        state = State::PROC;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::PROG:
                if (isAlphanumeric(c)) {
                    if (lower == 'r') {
                        adv();
                        state = State::PROGR;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::PROGR:
                if (isAlphanumeric(c)) {
                    if (lower == 'a') {
                        adv();
                        state = State::PROGRA;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::PROGRA:
                if (isAlphanumeric(c)) {
                    if (lower == 'm') {
                        adv();
                        state = State::PROGRAM;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::PROGRAM:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(PROGRAM, tokenStart, pos);
                }
                break;

            case State::PROC:
                if (isAlphanumeric(c)) {
                    if (lower == 'e') {
                        adv();
                        state = State::PROCE;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::PROCE:
                if (isAlphanumeric(c)) {
                    if (lower == 'd') {
                        adv();
                        state = State::PROCED;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::PROCED:
                if (isAlphanumeric(c)) {
                    if (lower == 'u') {
                        adv();
                        state = State::PROCEDU;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::PROCEDU:
                if (isAlphanumeric(c)) {
                    if (lower == 'r') {
                        adv();
                        state = State::PROCEDUR;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::PROCEDUR:
                if (isAlphanumeric(c)) {
                    if (lower == 'e') {
                        adv();
                        state = State::PROCEDURE;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::PROCEDURE:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(PROCEDURE, tokenStart, pos);
                }
                break;

            case State::R:
                if (isAlphanumeric(c)) {
                    if (lower == 'e') {
                        adv();
                        state = State::RE;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::RE:
                if (isAlphanumeric(c)) {
                    if (lower == 'c') {
                        adv();
                        state = State::REC;
                    } else if (lower == 'p') {
                        adv();
                        state = State::REP;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::REC:
                if (isAlphanumeric(c)) {
                    if (lower == 'o') {
                        adv();
                        state = State::RECO;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::RECO:
                if (isAlphanumeric(c)) {
                    if (lower == 'r') {
                        adv();
                        state = State::RECOR;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::RECOR:
                if (isAlphanumeric(c)) {
                    if (lower == 'd') {
                        adv();
                        state = State::RECORD;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::RECORD:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(RECORD, tokenStart, pos);
                }
                break;

            case State::REP:
                if (isAlphanumeric(c)) {
                    if (lower == 'e') {
                        adv();
                        state = State::REPE;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::REPE:
                if (isAlphanumeric(c)) {
                    if (lower == 'a') {
                        adv();
                        state = State::REPEA;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::REPEA:
                if (isAlphanumeric(c)) {
                    if (lower == 't') {
                        adv();
                        state = State::REPEAT;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::REPEAT:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(REPEAT, tokenStart, pos);
                }
                break;

            case State::T:
                if (isAlphanumeric(c)) {
                    if (lower == 'h') {
                        adv();
                        state = State::TH;
                    } else if (lower == 'o') {
                        adv();
                        state = State::TO;
                    } else if (lower == 'y') {
                        adv();
                        state = State::TY;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::TH:
                if (isAlphanumeric(c)) {
                    if (lower == 'e') {
                        adv();
                        state = State::THE;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::THE:
                if (isAlphanumeric(c)) {
                    if (lower == 'n') {
                        adv();
                        state = State::THEN;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::THEN:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(THEN, tokenStart, pos);
                }
                break;

            case State::TO:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(TO, tokenStart, pos);
                }
                break;

            case State::TY:
                if (isAlphanumeric(c)) {
                    if (lower == 'p') {
                        adv();
                        state = State::TYP;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;
            
            case State::TYP:
                if (isAlphanumeric(c)) {
                    if (lower == 'e') {
                        adv();
                        state = State::TYPE;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::TYPE:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(TYPE, tokenStart, pos);
                }
                break;

            case State::U:
                if (isAlphanumeric(c)) {
                    if (lower == 'n') {
                        adv();
                        state = State::UN;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::UN:
                if (isAlphanumeric(c)) {
                    if (lower == 't') {
                        adv();
                        state = State::UNT;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::UNT:
                if (isAlphanumeric(c)) {
                    if (lower == 'i') {
                        adv();
                        state = State::UNTI;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::UNTI:
                if (isAlphanumeric(c)) {
                    if (lower == 'l') {
                        adv();
                        state = State::UNTIL;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::UNTIL:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(UNTIL, tokenStart, pos);
                }
                break;

            case State::V:
                if (isAlphanumeric(c)) {
                    if (lower == 'a') {
                        adv();
                        state = State::VA;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::VA:
                if (isAlphanumeric(c)) {
                    if (lower == 'r') {
                        adv();
                        state = State::VAR;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::VAR:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(VAR, tokenStart, pos);
                }
                break;

            case State::W:
                if (isAlphanumeric(c)) {
                    if (lower == 'h') {
                        adv();
                        state = State::WH;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::WH:
                if (isAlphanumeric(c)) {
                    if (lower == 'i') {
                        adv();
                        state = State::WHI;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::WHI:
                if (isAlphanumeric(c)) {
                    if (lower == 'l') {
                        adv();
                        state = State::WHIL;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::WHIL:
                if (isAlphanumeric(c)) {
                    if (lower == 'e') {
                        adv();
                        state = State::WHILE;
                    } else {
                        adv();
                        state = State::IDENT;
                    }
                } else {
                    return makeToken(IDENTIFIER, tokenStart, pos);
                }
                break;

            case State::WHILE:
                if (isAlphanumeric(c)) {
                    adv();
                    state = State::IDENT;
                } else {
                    return makeToken(WHILE, tokenStart, pos);
                }
                break;
            default:
                throw runtime_error("Internal lexer state error");
        }
    }
}
