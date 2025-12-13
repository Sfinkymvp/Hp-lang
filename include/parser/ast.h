#ifndef AST_H_
#define AST_H_


#define PARSER_ASSERT                                                                  \
    assert(context); assert(context->symbols); assert(context->lexer_context);         \
    LEXER_ASSERT(context->lexer_context); 
typedef enum {
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER,
    SYMBOL_VARIABLE
} SymbolType;


typedef struct {
    SymbolType type;
    const char* text;
    size_t length;
} Symbol;


typedef struct {
    Symbol* symbols;
    size_t count;
    size_t capacity;
} SymbolTable;


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
    AST_NODE_IDENTIFIER

} AstNodeType;


typedef struct AstNode AstNode;
struct AstNode {
    AstNodeType type;
    AstNodeValue value;
    AstNode* left;
    AstNode* right;
    AstNode* parent;
};


typedef struct {
    AstNode root;
    SymbolTable symbol_table;
    LexerContext* lexer_context;
    size_t current_token;
    ParsingContextType current_parsing_context;
} ParserContext;


#endif // AST_H_