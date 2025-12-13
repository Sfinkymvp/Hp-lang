#ifndef LEXER_H_ 
#define LEXER_H_


#include "lexer/token.h"
#include "status.h"


OperationStatus createLexerContext(LexerContext* context, const char* filename);


void deleteLexerContext(LexerContext* context);


void runLexer(LexerContext* context);


#endif // LEXER_H_