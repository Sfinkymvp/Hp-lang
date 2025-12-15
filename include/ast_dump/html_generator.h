#ifndef HTML_GENERATOR_H_
#define HTML_GENERATOR_H_


#include "parser/ast.h"


#define AST_DUMP(context, ast_root, format, ...)   \
    visualizeAstTree(context, ast_root, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__)


void openAstDumpFile(ParserContext* context);


void visualizeAstTree(ParserContext* context, AstNode* ast_root, const char* file, 
    const char* function, int line, const char* format, ...);


#endif // HTML_GENERATOR_H_