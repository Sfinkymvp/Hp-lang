#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ast_shared.h"
#include "status.h"


const size_t BUFFER_SIZE = 32;


#define GENERATE_STRING_TYPE(type, symbol) {type, #type, symbol}


NodeTypeTable AST_NODE_TYPES_TABLE[] = {
    GENERATE_STRING_TYPE(AST_NODE_SEMICOLON,   ";"       ),
    GENERATE_STRING_TYPE(AST_NODE_SCOPE,       "{}"      ),
    GENERATE_STRING_TYPE(AST_NODE_CYCLE,       "while"   ),
    GENERATE_STRING_TYPE(AST_NODE_IF,          "if"      ),
    GENERATE_STRING_TYPE(AST_NODE_ELSE,        "else"    ),
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


const size_t TABLE_SIZE = sizeof(AST_NODE_TYPES_TABLE) / sizeof(AST_NODE_TYPES_TABLE[0]);


const char* getNodeType(AstNode* node)
{
    assert(node);

    for (size_t index = 0; index < TABLE_SIZE; index++) {
        if (node->type == AST_NODE_TYPES_TABLE[index].type) {
            return AST_NODE_TYPES_TABLE[index].display_name;
        }
    }

    return "UNKNOWN_NODE_TYPE";
}


const char* getNodeSymbol(IdentifierTable* id_table, AstNode* node)
{
    assert(id_table); assert(id_table->identifiers); assert(node);

    for (size_t index = 0; index < TABLE_SIZE; index++) {
        if (node->type == AST_NODE_TYPES_TABLE[index].type &&
            node->type != AST_NODE_NUMBER && node->type != AST_NODE_IDENTIFIER) {
            return AST_NODE_TYPES_TABLE[index].symbol;
        }
    }

    static char buffer[BUFFER_SIZE] = "";

    if (node->type == AST_NODE_NUMBER) {
        snprintf(buffer, BUFFER_SIZE, "%d", node->data.int_value);
        return buffer;
    } else if (node->type == AST_NODE_IDENTIFIER) {
        assert(node->data.id_index < id_table->count);
        return id_table->identifiers[node->data.id_index];
    } else {
        return "UNKNOWN_NODE_SYMBOL";
    }
}


OperationStatus createNode(AstNode** node)
{
    *node = (AstNode*)calloc(1, sizeof(AstNode));
    if (*node == NULL) {
        return STATUS_SYSTEM_OUT_OF_MEMORY;
    }

    return STATUS_OK;
}


void deleteSubtree(AstNode* node)
{
    if (node == NULL) {
        return;
    }

    if (node->left) {
        deleteSubtree(node->left);
        node->left = NULL;
    }
    if (node->right) {
        deleteSubtree(node->right);
        node->right = NULL;
    }
    free(node);
}

