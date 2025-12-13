#ifndef LEXER_ERROR_H_
#define LEXER_ERROR_H_


#include "lexer/token.h"


void reportLexerError(LexerContext* context, const char* error_start, size_t length,
    const char* error_message);


#endif // LEXER_ERROR_H_