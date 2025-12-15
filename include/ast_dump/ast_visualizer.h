#ifndef AST_VISUALIZER_H_
#define AST_VISUALIZER_H_


#include "parser/ast.h"


void generateGraph(ParserContext* context, AstNode* ast_root, const char* graph_filename);


#endif // AST_VISUALIZER_H_