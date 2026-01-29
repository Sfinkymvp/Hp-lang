#include <stdio.h>
#include <assert.h>

#include "io/writer.h"
#include "parser/ast.h"


OperationStatus writeAstTreeToDisk(ParserContext* context, AstNode* ast_root)
{
    PARSER_ASSERT(context); assert(ast_root);

    FILE* file = fopen(context->cmd_args.output_file, "w");
    if (file == NULL) {
        return STATUS_IO_FILE_OPEN_ERROR;
    }

    printAstTree(context, ast_root, file);

    if (fclose(file) != 0) {
        return STATUS_IO_FILE_CLOSE_ERROR;
    }

    return STATUS_OK;
}


void printAstTree(ParserContext* context, AstNode* node, FILE* file)
{
    PARSER_ASSERT(context); assert(node); assert(file);

    fprintf(file, "(");

    if (node->type == AST_NODE_IDENTIFIER) {
        fprintf(file, "\"%s\"", context->id_table.identifiers[node->data.id_index]);
    } else if (node->type == AST_NODE_NUMBER) {
        fprintf(file, "%d", node->data.int_value);
    } else {
        fprintf(file, "%s ", getNodeSymbol(context, node));
    }
    if (node->left) {
        printAstTree(context, node->left, file);
    } else {
        fprintf(file, " nil");
    } if (node->right) {
        printAstTree(context, node->right, file);
    } else {
        fprintf(file, " nil");
    }
        
    fprintf(file, ")");
}
