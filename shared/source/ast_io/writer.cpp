#include <stdio.h>
#include <assert.h>

#include "ast_io/writer.h"



OperationStatus writeAstTreeToDisk(IdentifierTable* id_table, AstNode* ast_root, const char* output_file)
{
    assert(id_table); assert(id_table->identifiers); assert(ast_root); assert(output_file);

    FILE* file = fopen(output_file, "w");
    if (file == NULL) {
        return STATUS_IO_FILE_OPEN_ERROR;
    }

    printAstTree(id_table, ast_root, file);

    if (fclose(file) != 0) {
        return STATUS_IO_FILE_CLOSE_ERROR;
    }

    return STATUS_OK;
}


void printAstTree(IdentifierTable* id_table, AstNode* node, FILE* file)
{
    assert(id_table); assert(id_table->identifiers); assert(node); assert(file);

    fprintf(file, "(");

    if (node->type == AST_NODE_IDENTIFIER) {
        fprintf(file, "%s", id_table->identifiers[node->data.id_index]);
    } else if (node->type == AST_NODE_NUMBER) {
        fprintf(file, "%d", node->data.int_value);
    } else {
        fprintf(file, "%s ", getNodeSymbol(id_table, node));
    }
    if (node->left) {
        printAstTree(id_table, node->left, file);
    } else {
        fprintf(file, " nil");
    } if (node->right) {
        printAstTree(id_table, node->right, file);
    } else {
        fprintf(file, " nil");
    }
        
    fprintf(file, ")");
}
