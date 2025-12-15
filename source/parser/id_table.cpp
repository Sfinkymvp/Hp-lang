#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parser/id_table.h"
#include "parser/ast.h"
#include "status.h"


void createIdentifierTable(ParserContext* context)
{
    char** temp = (char**)calloc(ID_TABLE_INITIAL_CAPACITY, sizeof(char*));
    if (temp == NULL) {
        context->status = STATUS_SYSTEM_OUT_OF_MEMORY;
        return;
    }

    context->id_table.identifiers = temp;
    context->id_table.count = 0;
    context->id_table.capacity = ID_TABLE_INITIAL_CAPACITY;
}


void expandIdentifierTable(ParserContext* context)
{
    PARSER_ASSERT(context);

    char** temp = (char**)realloc(context->id_table.identifiers, context->id_table.capacity * 2 * sizeof(char*));
    if (temp == NULL) {
        context->status = STATUS_SYSTEM_OUT_OF_MEMORY;
        return;
    }

    context->id_table.identifiers = temp;
    context->id_table.capacity *= 2;
}


size_t addIdentifier(ParserContext* context)
{
    PARSER_ASSERT(context);

    Token* token = &context->lexer_context.tokens_array.tokens[context->current_token];
    assert(token); assert(token->start); assert(token->type == TOKEN_IDENTIFIER);

    for (size_t index = 0; index < context->id_table.count; index++) {
        if (strncmp(token->start, context->id_table.identifiers[index], token->length) == 0 &&
            context->id_table.identifiers[index][token->length] == '\0') {
            return index;
        }
    }

    if (context->id_table.count == context->id_table.capacity) {
        expandIdentifierTable(context);
        if (context->status != STATUS_OK) {
            return (size_t)-1;
        }
    }

    size_t new_index = context->id_table.count;
    char* identifier = context->id_table.identifiers[new_index];

    identifier = (char*)calloc(token->length + 1, sizeof(char));
    if (identifier == NULL) {
        context->status = STATUS_SYSTEM_OUT_OF_MEMORY;
        return (size_t)-1;
    }

    memcpy(identifier, token->start, token->length);
    identifier[token->length] = '\0';

    context->id_table.identifiers[new_index] = identifier;
    context->id_table.count++;

    return new_index;
}


void deleteIdentifierTable(ParserContext* context)
{
    PARSER_ASSERT(context);

    for (size_t index = 0; index < context->id_table.count; index++) {
        free(context->id_table.identifiers[index]);
    }

    free(context->id_table.identifiers);
    context->id_table.count = 0;
    context->id_table.capacity = 0;
}