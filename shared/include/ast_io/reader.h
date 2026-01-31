#ifndef READER_H_
#define READER_H_


#include "ast_shared.h"
#include "id_table.h"


OperationStatus loadAstTreeFromDisk(IdentifierTable* id_table, AstNode** root, const char* input_file);


#endif // READER_H_