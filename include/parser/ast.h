#ifndef AST_H_
#define AST_H_


#include "status.h"
#include "lexer/token.h"


const size_t ID_TABLE_INITIAL_CAPACITY = 8;


#define PARSER_ASSERT(context)                                                         \
    assert(context); assert(context->id_table.identifiers);  


#define RETURN_IF_STATUS_NOT_OK(context)              \
    do {                                              \
        if ((context)->status != STATUS_OK) {         \
            return NULL;                              \
        }                                             \
    } while (0)


typedef enum {
    CONTEXT_GLOBAL,
    CONTEXT_FUNCTION,
    CONTEXT_LOOP
} ParsingContextType;


typedef enum {
    AST_NODE_SEMICOLON,
    AST_NODE_CYCLE,
    AST_NODE_IF,
    AST_NODE_DECLARATION,
    AST_NODE_ASSIGNMENT,
    AST_NODE_ARGUMENT,
    AST_NODE_CALL,
    AST_NODE_PARAMETER,
    AST_NODE_FUNCTION,
    AST_NODE_IDENTIFIER,
    AST_NODE_NUMBER,
    AST_NODE_OP_ADD,
    AST_NODE_OP_SUB,
    AST_NODE_OP_MUL,
    AST_NODE_OP_DIV,
    AST_NODE_OP_POW,
    AST_NODE_OP_NEG
} AstNodeType;


typedef struct {
    const char* input_file;
    const char* output_file;
    bool simple_visualizer;
} CmdArgs;


typedef struct {
    char** identifiers;
    size_t count;
    size_t capacity;
} IdentifierTable;


typedef union {
    size_t id_index;
    int int_value;
} NodeData;


typedef struct AstNode AstNode;
struct AstNode {
    AstNodeType type;
    NodeData data;
    AstNode* left;
    AstNode* right;
    AstNode* parent;
};


typedef struct {
    CmdArgs cmd_args;
    FILE* dump_file;
    IdentifierTable id_table;
    LexerContext lexer_context;
    size_t current_token;
    ParsingContextType current_parsing_context;
    OperationStatus status;
} ParserContext;


#endif // AST_H_