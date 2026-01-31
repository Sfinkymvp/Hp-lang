#ifndef HTML_GENERATOR_H_
#define HTML_GENERATOR_H_


#include "parser/ast.h"


typedef struct {
    IdentifierTable* id_table;
    FILE* dump_file;
    bool simple_visualizer;
} VisualizerContext;


FILE* openAstDumpFile();


void visualizeAstTree(VisualizerContext context, AstNode* ast_root, const char* file, 
    const char* function, int line, const char* format, ...);


#endif // HTML_GENERATOR_H_