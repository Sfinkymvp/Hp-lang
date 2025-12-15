#ifndef TOKEN_HANDLERS_H_
#define TOKEN_HANDLERS_H_


#include "lexer/token.h"


typedef struct {
    TokenType type;
    const char* text;
    size_t length;
} KeyWord;


extern const KeyWord KEYWORD_TABLE[];
extern const size_t KEYWORD_TABLE_SIZE;


const char* getKeyWordString(TokenType type);


#endif // TOKEN_HANDLERS_H_