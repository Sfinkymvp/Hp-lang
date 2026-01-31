#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parser/ast.h"
#include "status.h"


const size_t BUFFER_SIZE = 32;


#define GENERATE_STRING_TYPE(type, symbol) {type, #type, symbol}


AstNodeTypeInfo AST_NODE_TYPES_STRINGS[] = {
    GENERATE_STRING_TYPE(AST_NODE_SEMICOLON,   ";"       ),
    GENERATE_STRING_TYPE(AST_NODE_SCOPE,       "{}"      ),
    GENERATE_STRING_TYPE(AST_NODE_CYCLE,       "while"   ),
    GENERATE_STRING_TYPE(AST_NODE_IF,          "if"      ),
    GENERATE_STRING_TYPE(AST_NODE_DECLARATION, ":="      ),
    GENERATE_STRING_TYPE(AST_NODE_ASSIGNMENT,  "="       ),
    GENERATE_STRING_TYPE(AST_NODE_COMMA,       ","       ),
    GENERATE_STRING_TYPE(AST_NODE_CALL,        "call"    ),
    GENERATE_STRING_TYPE(AST_NODE_FUNCTION,    "func"    ),
    GENERATE_STRING_TYPE(AST_NODE_RETURN,      "return"  ),
    GENERATE_STRING_TYPE(AST_NODE_IDENTIFIER,  ""        ),
    GENERATE_STRING_TYPE(AST_NODE_NUMBER,      ""        ),
    GENERATE_STRING_TYPE(AST_NODE_OP_ADD,      "+"       ),
    GENERATE_STRING_TYPE(AST_NODE_OP_SUB,      "-"       ),
    GENERATE_STRING_TYPE(AST_NODE_OP_MUL,      "*"       ),
    GENERATE_STRING_TYPE(AST_NODE_OP_DIV,      "/"       ),
    GENERATE_STRING_TYPE(AST_NODE_OP_POW,      "^"       ),
    GENERATE_STRING_TYPE(AST_NODE_OP_NEG,      "-"       )
};


const size_t STRINGS_SIZE = sizeof(AST_NODE_TYPES_STRINGS) / sizeof(AST_NODE_TYPES_STRINGS[0]);



const char* getNodeType(ParserContext* context, AstNode* node)
{
    assert(context); assert(node);

    for (size_t index = 0; index < STRINGS_SIZE; index++) {
        if (node->type == AST_NODE_TYPES_STRINGS[index].type) {
            return AST_NODE_TYPES_STRINGS[index].display_name;
        }
    }

    return "UNKNOWN_NODE_TYPE";
}


const char* getNodeSymbol(ParserContext* context, AstNode* node)
{
    assert(context); assert(node);

    for (size_t index = 0; index < STRINGS_SIZE; index++) {
        if (node->type == AST_NODE_TYPES_STRINGS[index].type &&
            node->type != AST_NODE_NUMBER && node->type != AST_NODE_IDENTIFIER) {
            return AST_NODE_TYPES_STRINGS[index].symbol;
        }
    }

    static char buffer[BUFFER_SIZE] = "";

    if (node->type == AST_NODE_NUMBER) {
        snprintf(buffer, BUFFER_SIZE, "%d", node->data.int_value);
        return buffer;
    } else if (node->type == AST_NODE_IDENTIFIER) {
        assert(node->data.id_index < context->id_table.count);
        return context->id_table.identifiers[node->data.id_index];
    } else {
        return "UNKNOWN_NODE_SYMBOL";
    }
}


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

    Token* token = &context->tokens_array->tokens[context->current_token];
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
    assert(context);

    for (size_t index = 0; index < context->id_table.count; index++) {
        free(context->id_table.identifiers[index]);
    }

    free(context->id_table.identifiers);
    context->id_table.count = 0;
    context->id_table.capacity = 0;
}