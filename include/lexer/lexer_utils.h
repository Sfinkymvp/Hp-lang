#ifndef LEXER_UTILS_H_ 
#define LEXER_UTILS_H_


#include "lexer/token.h"
#include "status.h"


OperationStatus createLexerContext(LexerContext* context, const char* filename);


void deleteLexerContext(LexerContext* context);


void reportLexerError(LexerContext* context, const char* error_start, size_t length,
    const char* error_message);


#endif // LEXER_UTILS_H_