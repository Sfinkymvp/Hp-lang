#ifndef TOKEN_H_
#define TOKEN_H_


const size_t TOKEN_INITIAL_CAPACITY = 64;


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
    TOKEN_FUNC,
    TOKEN_PARAM,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_CYCLE,
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_UNKNOWN
} TokenType;


typedef struct {
    TokenType type;
    const char* text;
    size_t length;
} KeyWord;


typedef struct {
    const char* start;
    size_t length;
} LineInfo;


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


#define LEXER_ASSERT(context)                                                                    \
    assert(context->source_map); assert(context->source_map->buffer);                            \
    assert(context->source_map->lines); assert(context->tokens_array.tokens);                    \
    assert(context->current); assert(context->current_line <= context->source_map->lines_count)


extern const KeyWord KEYWORD_TABLE[];
extern const size_t KEYWORD_TABLE_SIZE;


const char* getKeywordString(TokenType type);


#endif // TOKEN_H_