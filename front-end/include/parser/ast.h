#ifndef AST_H_
#define AST_H_


#include "ast_types.h"
#include "status.h"
#include "lexer/token.h"


const size_t ID_TABLE_INITIAL_CAPACITY = 8;


#define PARSER_ASSERT(context)                                \
    assert(context); assert(context->id_table.identifiers);  


#define RETURN_IF_STATUS_NOT_OK(context)              \
    do {                                              \
        if ((context)->status != STATUS_OK) {         \
            return NULL;                              \
        }                                             \
    } while (0)


typedef enum {
    SCOPE_GLOBAL,
    SCOPE_FUNCTION,
    SCOPE_BLOCK
} ScopeType;


typedef struct {
    const char* input_file;
    const char* output_file;
    bool simple_visualizer;
} CmdArgs;


typedef struct {
    AstNodeType type;
    const char* display_name;
    const char* symbol;
} AstNodeTypeInfo;


extern AstNodeTypeInfo AST_NODE_TYPES_STRINGS[];
extern const size_t STRINGS_SIZE;


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
    ScopeType current_scope;
    OperationStatus status;
} ParserContext;


const char* getNodeType(ParserContext* context, AstNode* node);


const char* getNodeSymbol(ParserContext* context, AstNode* node);

 
void createIdentifierTable(ParserContext* context);


void expandIdentifierTable(ParserContext* context);


size_t addIdentifier(ParserContext* context);


void deleteIdentifierTable(ParserContext* context);


#endif // AST_H_