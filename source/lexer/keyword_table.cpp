#include <stdio.h>
#include <assert.h>

#include "lexer/token.h"


const KeyWord KEYWORD_TABLE[] = {
    {TOKEN_OP_ADD,         "+",      1},
    {TOKEN_OP_SUB,         "-",      1},
    {TOKEN_OP_MUL,         "*",      1},
    {TOKEN_OP_DIV,         "/",      1},
    {TOKEN_OP_POW,         "^",      1},
    {TOKEN_OP_ASSIGN,      "=",      1},
    {TOKEN_OP_DECLARATION, ":=",     2},
    {TOKEN_COMMA,          ",",      1},
    {TOKEN_LEFT_PAREN,     "(",      1},
    {TOKEN_RIGHT_PAREN,    ")",      1},
    {TOKEN_SEMICOLON,      ";",      1},
    {TOKEN_LEFT_BRACE,     "{",      1},
    {TOKEN_RIGHT_BRACE,    "}",      1},
    {TOKEN_FUNC,           "func",   4},
    {TOKEN_PARAM,          "param",  5},
    {TOKEN_RETURN,         "return", 6},
    {TOKEN_CYCLE,          "while",  5},
    {TOKEN_IF,             "if",     2},
};


const size_t KEYWORD_TABLE_SIZE = sizeof(KEYWORD_TABLE) / sizeof(KEYWORD_TABLE[0]);


const char* getKeywordString(TokenType type) 
{
    for (size_t index = 0; index < KEYWORD_TABLE_SIZE; index++) {
        if (type == KEYWORD_TABLE[index].type) {
            return KEYWORD_TABLE[index].text;
        }
    }

    return NULL;
}