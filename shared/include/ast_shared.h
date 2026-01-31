#ifndef AST_SHARED_H_
#define AST_SHARED_H_


#include "id_table.h"


typedef enum {
    AST_NODE_SEMICOLON = 0,
    AST_NODE_SCOPE,
    AST_NODE_CYCLE,
    AST_NODE_IF,
    AST_NODE_ELSE,
    AST_NODE_DECLARATION,
    AST_NODE_ASSIGNMENT,
    AST_NODE_COMMA,
    AST_NODE_CALL,
    AST_NODE_FUNCTION,
    AST_NODE_RETURN,
    AST_NODE_IDENTIFIER,
    AST_NODE_NUMBER,
    AST_NODE_OP_ADD,
    AST_NODE_OP_SUB,
    AST_NODE_OP_MUL,
    AST_NODE_OP_DIV,
    AST_NODE_OP_POW,
    AST_NODE_OP_NEG
} NodeType;


typedef struct {
    NodeType type;
    const char* display_name;
    const char* symbol;
} NodeTypeTable;


extern NodeTypeTable AST_NODE_TYPES_TABLE[];
extern const size_t TABLE_SIZE;


typedef union {
    size_t id_index;
    int int_value;
} NodeData;


typedef struct AstNode AstNode;
struct AstNode {
    NodeType type;
    NodeData data;
    size_t line;
    AstNode* left;
    AstNode* right;
    AstNode* parent;
};


const char* getNodeType(AstNode* node);


const char* getNodeSymbol(IdentifierTable* id_table, AstNode* node);


OperationStatus createNode(AstNode** node);


void deleteSubtree(AstNode* node);


#endif // AST_SHARED_H_