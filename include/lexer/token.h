#ifndef TOKEN_H_
#define TOKEN_H_


const size_t TOKEN_INITIAL_CAPACITY = 64;


#define LEXER_ASSERT(context)                                                                  \
    assert(context->source_map); assert(context->source_map->buffer);         \
    assert(context->source_map->lines); assert(context->tokens_array.tokens);                  \
    assert(context->current); assert(context->current_line <= context->source_map->lines_count)


typedef enum {
    TOKEN_EOF,
    TOKEN_OP_ADD,
    TOKEN_OP_SUB,
    TOKEN_OP_MUL,
    TOKEN_OP_DIV,
    TOKEN_OP_POW,
    TOKEN_OP_ASSIGN,
    TOKEN_OP_DECLARATION,
    TOKEN_COMMA,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_SEMICOLON,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_KEYWORD_FUNC,
    TOKEN_KEYWORD_PARAM,
    TOKEN_KEYWORD_RETURN,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_CYCLE,
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_UNKNOWN
} TokenType;


typedef struct {
    TokenType type;
    const char* start;
    size_t length;
    size_t line;
} Token;


typedef struct {
    Token* tokens;
    size_t count;
    size_t capacity;
} TokensArray;


typedef struct {
    const char* start;
    size_t length;
} LineInfo;


typedef struct {
    char* buffer;
    LineInfo* lines;
    size_t lines_count;
} SourceMap;


typedef struct {
    SourceMap* source_map;
    TokensArray tokens_array;
    const char* current;
    size_t current_line;
} LexerContext;


#endif // TOKEN_H_