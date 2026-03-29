#include "Lexer.hpp"

const unordered_map<string, TokenType> Lexer::keywords = {
    {"div", IDIV},
    {"mod", MOD},
    {"const", CONST},
    {"type", TYPE},
    {"var", VAR},
    {"function", FUNCTION},
    {"procedure", PROCEDURE},
    {"array", ARRAY},
    {"record", RECORD},
    {"program", PROGRAM},

    {"begin", BEGIN},
    {"end", END},
    {"if", IF},
    {"then", THEN},
    {"else", ELSE},
    {"case", CASE},
    {"of", OF},
    {"repeat", REPEAT},
    {"until", UNTIL},
    {"while", WHILE},
    {"do", DO},
    {"for", FOR},
    {"to", TO},
    {"downto", DOWNTO},

    {"div", IDIV},
    {"mod", MOD},
    {"and", AND},
    {"or", OR},
    {"not", NOT}};

Lexer::Lexer(const string& input) : line(input), pos(0) {}

char Lexer::peek(int offset){
    if (pos + offset >= line.size()) return '\0';
    return line[pos + offset];
}

char Lexer::adv(){
    if (pos >= line.size()) return '\0';
    return line[pos++];
}

bool Lexer::isEnd(){
    return pos >= line.size();
}

bool Lexer::isAlphabet(char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::isDigit(char c){
    return c >= '0' && c <= '9';
}

bool Lexer::isAlphanumeric(char c){
    return isAlphabet(c) || isDigit(c);
}

bool Lexer::isSpace(char c){
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void Lexer::skip(){
    while (!isEnd()){
        if (isSpace(peek(0))){
            adv();
        } else if (peek(0) == '{') { // komen
            adv();
            while (!isEnd() && peek(0)!= '}') adv();
            if (isEnd()){
                throw runtime_error("Unclosed comment");
            }
            adv();
        } else {
            if (peek(0) == '(' && peek(1) == '*'){
                adv();
                adv();
                while (!isEnd()){
                    if (peek(0)=='*' && peek(1)==')'){
                        adv();
                        adv();
                        break;
                    }
                    adv();
                }
            }
            else break;
        }
    }
}

string Lexer::toLower(const string& str){
    string input = str;
    for (int i = 0; i < input.length(); i++){
        if (input[i] >= 'A' && input[i] <= 'Z'){
            input[i] += 32;
        }
    }
    return input;
}

vector<Token> Lexer::tokenize(){
    vector<Token> tokens;
    while (!isEnd()){
        skip();
        if (isEnd()) break;
        tokens.push_back(nextToken());
    }
    return tokens;
}

Token Lexer::handleSymbol(){
    char c = peek(0);
    switch(c){
        case '+':
            adv();
            return {PLUS, "+"};
        case '-':
            adv();
            return {MINUS, "-"};
        case '*':
            adv();
            return {TIMES, "*"};
        case '/':
            adv();
            return {RDIV, "/"};
        case ':':
            adv();
            if (peek(0)=='='){
                adv();
                return {ASSIGN, ":="};
            }
            return {COLON, ":"};
        case '<':
            adv();
            if (peek(0)=='='){
                adv();
                return {LEQ, "<="};
            }
            if (peek(0)=='>'){
                adv();
                return {NEQ, "<>"};
            }
            return {LSS, "<"};
        case '>':
            adv();
            if (peek(0)=='='){
                adv();
                return {GEQ, ">="};
            }
            return {GTR, ">"};
        case '=':
            if (peek(1)=='='){
                adv();
                adv();
                return {EQL, "=="};
            }
            throw runtime_error("'=' must be followed by another '='");
        case '(':
            adv();
            return {LPARENT, "("};
        case ')':
            adv();
            return {RPARENT, ")"};
        case '[':
            adv();
            return {LBRACK, "["};
        case ']':
            adv();
            return {RBRACK, "]"};
        case ';':
            adv();
            return {SEMICOLON, ";"};
        case '.':
            adv();
            return {PERIOD, "."};
        case ',':
            adv();
            return {COMMA, ","};
        default : 
            throw runtime_error("Unknown symbol: " + string(1, c));
    }   
}

Token Lexer::nextToken(){
    State state = State::START;
    string value = "";

    while (true){
        char c = peek(0);
        switch(state){
            case State::START:
                if (isAlphabet(c)){
                    value += adv();
                    state = State::IDENT;
                } else if (isDigit(c)){
                    value += adv();
                    state = State::INT;
                } else if (c == '\''){
                    adv();
                    state = State::STRING;
                } else {
                    return handleSymbol();
                }
                break;
            case State::IDENT:
                while (isAlphanumeric(peek(0))) {
                    value += adv();
                }
            
                {
                    string lower = toLower(value);
            
                    if (keywords.count(lower)) {
                        return {keywords.at(lower), value};
                    }
            
                    return {IDENTIFIER, value};
                }
                break;
        } // LANJUT DARI SINI HARUS NYA YANG LAIN 
    }
}