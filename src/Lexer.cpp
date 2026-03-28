#include "Lexer.hpp"

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
                cout << "Unclosed comment" << endl;
                return;
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
    string input;
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