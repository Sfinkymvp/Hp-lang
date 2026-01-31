#ifndef PARSER_UTILS_H_
#define PARSER_UTILS_H_


#include "status.h"
#include "lexer/utils.h"
#include "parser/ast.h"


void createParserContext(ParserContext* parser, LexerContext* lexer);


void deleteParserContext(ParserContext* context);


void reportParserError(ParserContext* context, TokenType expected_type, const char* error_message);


void deleteSubtree(AstNode* node);


AstNode* makeNode(ParserContext* context, AstNodeType type, size_t line, AstNode* left, AstNode* right);


#endif // PARSER_UTILS_H_