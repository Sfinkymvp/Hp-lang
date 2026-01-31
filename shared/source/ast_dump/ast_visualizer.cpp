#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ast_dump/ast_visualizer.h"
#include "lexer/lexer.h"
#include "parser/parser.h"


static void generateNode(IdentifierTable* id_table, AstNode* node, FILE* graph_file, int rank, int* counter);
static void printNodeAttributes(IdentifierTable* id_table, AstNode* node, FILE* graph_file, int id);
static void generateSimpleNode(IdentifierTable* id_table, AstNode* node, FILE* graph_file, int rank, int* counter);
static void printSimpleNodeAttributes(IdentifierTable* id_table, AstNode* node, FILE* graph_file, int id);


void generateGraph(VisualizerContext* context, AstNode* ast_root, const char* graph_filename)
{
    assert(context); assert(context->id_table); assert(ast_root); assert(graph_filename);

    FILE* graph_file = fopen(graph_filename, "w");
    assert(graph_file);

    fprintf(graph_file,
        "digraph AstTree {\n"
        "\trankdir=TB\n"
        "\tgraph[splines=line];\n");

    int counter = 0;
    int rank = 0;
    if (context->simple_visualizer) {
        generateSimpleNode(context->id_table, ast_root, graph_file, rank, &counter);
    } else {
        generateNode(context->id_table, ast_root, graph_file, rank, &counter);
    }
    fprintf(graph_file, "}\n\n");

    assert(fclose(graph_file) == 0);
}


static void generateNode(IdentifierTable* id_table, AstNode* node, FILE* graph_file, int rank, int* counter)
{
    assert(id_table); assert(node); assert(graph_file); assert(counter);
   
    int id = ++(*counter); 

    printNodeAttributes(id_table, node, graph_file, id);
    if (node->left) {
        fprintf(graph_file, "\tnode_%d:left -> node_%d:n [rank=%d];\n", 
                id, *counter + 1, rank);
        generateNode(id_table, node->left, graph_file, rank + 1, counter);
    }
    if (node->right) {
        fprintf(graph_file, "\tnode_%d:right -> node_%d:n [rank=%d];\n", 
                id, *counter + 1, rank);
        generateNode(id_table, node->right, graph_file, rank + 1, counter);
    }
}


static void printNodeAttributes(IdentifierTable* id_table, AstNode* node, FILE* graph_file, int id)
{
    assert(id_table); assert(node); assert(graph_file);

    fprintf(graph_file,
        "\tnode_%d [shape=Mrecord, fontname=\"Monospace\", "
        "penwidth=2.0, style=filled, label="
        "\"{<pointer>%p | parent: %p | Type: %s | Data: %s", id, node, node->parent, getNodeType(node),
        getNodeSymbol(id_table, node));

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


static void generateSimpleNode(IdentifierTable* id_table, AstNode* node, FILE* graph_file, int rank, int* counter)
{
    assert(id_table); assert(node); assert(graph_file); assert(counter);
  
    int id = ++(*counter); 
    printSimpleNodeAttributes(id_table, node, graph_file, id);
    if (node->left) {
        fprintf(graph_file, "\tnode_%d -> node_%d [rank=%d];\n", id, *counter + 1, rank);
        generateSimpleNode(id_table, node->left, graph_file, rank + 1, counter);
    }
    if (node->right) {
        fprintf(graph_file, "\tnode_%d -> node_%d [rank=%d];\n", id, *counter + 1, rank);
        generateSimpleNode(id_table, node->right, graph_file, rank + 1, counter);
    }
}


static void printSimpleNodeAttributes(IdentifierTable* id_table, AstNode* node, FILE* graph_file, int id)
{
    assert(id_table); assert(node); assert(graph_file);

    fprintf(graph_file,
        "\tnode_%d [shape=\"box\", fontname=\"Monospace\", "
        "penwidth=2.0, style=filled, label=\"%s\"];\n", id, getNodeSymbol(id_table, node));
}