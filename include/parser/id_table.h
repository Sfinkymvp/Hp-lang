#ifndef ID_TABLE_H_
#define ID_TABLE_H_


#include "parser/ast.h"
#include "status.h"


void createIdentifierTable(ParserContext* context);


void expandIdentifierTable(ParserContext* context);


size_t addIdentifier(ParserContext* context);


void deleteIdentifierTable(ParserContext* context);


#endif // ID_TABLE_H_
