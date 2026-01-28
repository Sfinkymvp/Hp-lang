#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ast_dump/ast_visualizer.h"
#include "lexer/token.h"
#include "lexer/keyword_table.h"
#include "parser/ast.h"


static void generateNode(ParserContext* context, AstNode* node,
    FILE* graph_file, int rank, int* counter);
static void printNodeAttributes(ParserContext* context, AstNode* node, FILE* graph_file, int id);

static void generateSimpleNode(ParserContext* context, AstNode* node,
    FILE* graph_file, int rank, int* counter);
static void printSimpleNodeAttributes(ParserContext* context, AstNode* node, FILE* graph_file, int id);


void generateGraph(ParserContext* context, AstNode* ast_root, const char* graph_filename)
{
    PARSER_ASSERT(context); assert(ast_root); assert(graph_filename);

    FILE* graph_file = fopen(graph_filename, "w");
    assert(graph_file);

    fprintf(graph_file,
        "digraph AstTree {\n"
        "\trankdir=TB\n"
        "\tgraph[splines=line];\n");

    int counter = 0;
    int rank = 0;
    if (context->cmd_args.simple_visualizer) {
        generateSimpleNode(context, ast_root, graph_file, rank, &counter);
    } else {
        generateNode(context, ast_root, graph_file, rank, &counter);
    }
    fprintf(graph_file, "}\n\n");

    assert(fclose(graph_file) == 0);
}


static void generateNode(ParserContext* context, AstNode* node,
    FILE* graph_file, int rank, int* counter)
{
    assert(context); assert(node); assert(graph_file); assert(counter);
   
    int id = ++(*counter); 

    printNodeAttributes(context, node, graph_file, id);
    if (node->left) {
        fprintf(graph_file, "\tnode_%d:left -> node_%d:n [rank=%d];\n", 
                id, *counter + 1, rank);
        generateNode(context, node->left, graph_file, rank + 1, counter);
    }
    if (node->right) {
        fprintf(graph_file, "\tnode_%d:right -> node_%d:n [rank=%d];\n", 
                id, *counter + 1, rank);
        generateNode(context, node->right, graph_file, rank + 1, counter);
    }
}


static void printNodeAttributes(ParserContext* context, AstNode* node, FILE* graph_file, int id)
{
    assert(context); assert(node); assert(graph_file);

    fprintf(graph_file,
        "\tnode_%d [shape=Mrecord, fontname=\"Monospace\", "
        "penwidth=2.0, style=filled, label="
        "\"{<pointer>%p | parent: %p | Type: %s | Data: %s", id, node, node->parent, getNodeType(context, node),
        getNodeSymbol(context, node));

    if (node->left && node->right) {
        fprintf(graph_file, " | {<left>%p | <right>%p", node->left, node->right);
    } else if (node->left) {
        fprintf(graph_file, " | {<left>%p | <right>nil", node->left);
    } else if (node->right) {
        fprintf(graph_file, " | {<left>nil | <right>%p", node->right);
    } else {
        fprintf(graph_file, " | {<left>nil | <right>nil");
    }
    fprintf(graph_file, "}}\"];\n");

}


static void generateSimpleNode(ParserContext* context, AstNode* node,
    FILE* graph_file, int rank, int* counter)
{
    assert(context); assert(node); assert(graph_file); assert(counter);
  
    int id = ++(*counter); 
    printSimpleNodeAttributes(context, node, graph_file, id);
    if (node->left) {
        fprintf(graph_file, "\tnode_%d -> node_%d [rank=%d];\n", id, *counter + 1, rank);
        generateSimpleNode(context, node->left, graph_file, rank + 1, counter);
    }
    if (node->right) {
        fprintf(graph_file, "\tnode_%d -> node_%d [rank=%d];\n", id, *counter + 1, rank);
        generateSimpleNode(context, node->right, graph_file, rank + 1, counter);
    }
}


static void printSimpleNodeAttributes(ParserContext* context, AstNode* node, FILE* graph_file, int id)
{
    assert(context); assert(node); assert(graph_file);

    fprintf(graph_file,
        "\tnode_%d [shape=\"box\", fontname=\"Monospace\", "
        "penwidth=2.0, style=filled, label=\"%s\"];\n", id, getNodeSymbol(context, node));
}
