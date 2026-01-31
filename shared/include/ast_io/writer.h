#ifndef WRITER_H_
#define WRITER_H_


#include "ast_shared.h"
#include "id_table.h"
#include "status.h"


OperationStatus writeAstTreeToDisk(IdentifierTable* id_table, AstNode* ast_root, const char* output_file);


void printAstTree(IdentifierTable* id_table, AstNode* node, FILE* file);


#endif // WRITER_H_