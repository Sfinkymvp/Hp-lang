#ifndef PARSER_UTILS_H_
#define PARSER_UTILS_H_


#include "status.h"
#include "lexer/lexer_utils.h"
#include "parser/ast.h"


void createParserContext(ParserContext* context, const int argc, const char** argv);


void deleteParserContext(ParserContext* context);


void reportParserError(ParserContext* context, TokenType expected_type, const char* error_message);


void deleteSubtree(AstNode* node);


AstNode* makeNode(ParserContext* context, AstNodeType type, AstNode* left, AstNode* right);


#endif // PARSER_UTILS_H_