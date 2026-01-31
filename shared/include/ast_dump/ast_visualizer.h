#ifndef AST_VISUALIZER_H_
#define AST_VISUALIZER_H_


#include "ast_shared.h"


typedef struct {
    IdentifierTable* id_table;
    FILE* dump_file;
    bool simple_visualizer;
} VisualizerContext;


void generateGraph(VisualizerContext* context, AstNode* ast_root, const char* graph_filename);


#endif // AST_VISUALIZER_H_