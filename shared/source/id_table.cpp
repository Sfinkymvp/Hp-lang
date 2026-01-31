#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "id_table.h"
#include "status.h"


OperationStatus createIdentifierTable(IdentifierTable* id_table)
{
    assert(id_table);

    char** temp = (char**)calloc(ID_TABLE_INITIAL_CAPACITY, sizeof(char*));
    if (temp == NULL) {
        return STATUS_SYSTEM_OUT_OF_MEMORY;
    }

    id_table->identifiers = temp;
    id_table->count = 0;
    id_table->capacity = ID_TABLE_INITIAL_CAPACITY;

    return STATUS_OK;
}


OperationStatus expandIdentifierTable(IdentifierTable* id_table)
{
    assert(id_table);

    char** temp = (char**)realloc(id_table->identifiers, id_table->capacity * 2 * sizeof(char*));
    if (temp == NULL) {
        return STATUS_SYSTEM_OUT_OF_MEMORY;
    }

    id_table->identifiers = temp;
    id_table->capacity *= 2;

    return STATUS_OK;
}


void deleteIdentifierTable(IdentifierTable* id_table)
{
    assert(id_table);

    for (size_t index = 0; index < id_table->count; index++) {
        free(id_table->identifiers[index]);
    }

    free(id_table->identifiers);
    id_table->count = 0;
    id_table->capacity = 0;
}