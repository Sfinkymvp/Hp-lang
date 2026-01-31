#ifndef ID_TABLE_H_
#define ID_TABLE_H_


#include <stdio.h>

#include "status.h"


const size_t ID_TABLE_INITIAL_CAPACITY = 8;


typedef struct {
    char** identifiers;
    size_t count;
    size_t capacity;
} IdentifierTable;


OperationStatus createIdentifierTable(IdentifierTable* id_table);


OperationStatus addIdentifier(IdentifierTable* id_table, const char* identifier, size_t* id_index);


OperationStatus expandIdentifierTable(IdentifierTable* id_table);


void deleteIdentifierTable(IdentifierTable* id_table);


#endif // ID_TABLE_H_