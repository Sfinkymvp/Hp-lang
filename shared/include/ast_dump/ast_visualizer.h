#ifndef AST_VISUALIZER_H_
#define AST_VISUALIZER_H_


#include "parser/parser.h"
#include "ast_dump/html_generator.h"


void generateGraph(VisualizerContext* context, AstNode* ast_root, const char* graph_filename);


#endif // AST_VISUALIZER_H_