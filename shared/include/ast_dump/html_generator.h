#ifndef HTML_GENERATOR_H_
#define HTML_GENERATOR_H_


#include "ast_shared.h"
#include "ast_visualizer.h"


FILE* openAstDumpFile();


void visualizeAstTree(VisualizerContext context, AstNode* ast_root, const char* file, 
    const char* function, int line, const char* format, ...);


#endif // HTML_GENERATOR_H_