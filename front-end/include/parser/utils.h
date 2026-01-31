#ifndef PARSER_UTILS_H_
#define PARSER_UTILS_H_


#include "status.h"
#include "lexer/utils.h"
#include "parser/ast.h"


#define CREATE_VIZ(context) ((VisualizerContext){                      \
    .id_table = &(context)->id_table,                                   \
    .dump_file = (context)->dump_file,                                  \
    .simple_visualizer = (context)->args->simple_visualizer})


#define AST_DUMP(context, ast_root, format, ...)                 \
    visualizeAstTree(CREATE_VIZ(context), ast_root,              \
    __FILE__, __func__, __LINE__, format, ##__VA_ARGS__)


void createParserContext(ParserContext* parser, LexerContext* lexer);


void deleteParserContext(ParserContext* context);


void reportParserError(ParserContext* context, TokenType expected_type, const char* error_message);


AstNode* makeNode(ParserContext* context, NodeType type, size_t line, AstNode* left, AstNode* right);


#endif // PARSER_UTILS_H_