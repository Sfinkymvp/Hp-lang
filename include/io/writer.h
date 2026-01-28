#ifndef WRITER_H_
#define WRITER_H_


#include "parser/ast.h"
#include "status.h"


OperationStatus writeAstTreeToDisk(ParserContext* context, AstNode* ast_root);


void printAstTree(ParserContext* context, AstNode* node, FILE* file);


#endif // WRITER_H_