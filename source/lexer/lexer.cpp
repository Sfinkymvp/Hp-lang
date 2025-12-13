#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "lexer/lexer.h"
#include "lexer/token.h"
#include "lexer/lexer_error.h"
#include "io/reader.h"


KeyWord KEYWORDS_TABLE[] = {
    {TOKEN_KEYWORD_FUNC,   "func",   4},
    {TOKEN_KEYWORD_PARAM,  "param",  5},
    {TOKEN_KEYWORD_VAR,    "var",    3},
    {TOKEN_KEYWORD_RETURN, "return", 6},
    {TOKEN_KEYWORD_CYCLE,  "while",  5},
    {TOKEN_KEYWORD_IF,     "if",     2}
};


const size_t KEYWORDS_LENGTH = sizeof(KEYWORDS_TABLE) / sizeof(KEYWORDS_TABLE[0]);


static void expandTokensArray(LexerContext* context);
static void parseSpaces(LexerContext* context);
static void parseEof(LexerContext* context);
static void parseLeftParen(LexerContext* context);
static void parseRightParen(LexerContext* context);
static void parseSemicolon(LexerContext* context);
static void parseLeftBrace(LexerContext* context);
static void parseRightBrace(LexerContext* context);
static void parseOpAdd(LexerContext* context);
static void parseOpSub(LexerContext* context);
static void parseOpMul(LexerContext* context);
static void parseOpDiv(LexerContext* context);
static void parseOpAssign(LexerContext* context);
static void parseNumber(LexerContext* context);
static void parseIdentifier(LexerContext* context);
static void parseError(LexerContext* context);
static bool isValidToken(char token);


OperationStatus createLexerContext(LexerContext* context, const char* filename)
{
    assert(context); assert(filename);

    SourceMap* source_map = (SourceMap*)calloc(1, sizeof(SourceMap));
    if (source_map == NULL) {
        return STATUS_SYSTEM_OUT_OF_MEMORY;
    }
    OperationStatus status = readSourceFile(source_map, filename);
    if (status != STATUS_OK) {
        free(source_map);
        return status;
    }
    Token* tokens = (Token*)calloc(TOKEN_INITIAL_CAPACITY, sizeof(Token));
    if (tokens == NULL) {
        deleteSourceMap(source_map);
        free(source_map);
        return STATUS_SYSTEM_OUT_OF_MEMORY;
    }

    context->tokens_array.count = 0;
    context->tokens_array.capacity = TOKEN_INITIAL_CAPACITY;
    context->tokens_array.tokens = tokens;

    context->source_map = source_map;
    context->current = context->source_map->buffer;
    context->current_line = 1;

    return STATUS_OK;
}


void deleteLexerContext(LexerContext* context)
{
    assert(context);

    if (context->source_map) {
        deleteSourceMap(context->source_map);
        free(context->source_map);
        context->source_map = NULL;
    }
    if (context->tokens_array.tokens) {
        free(context->tokens_array.tokens);
        context->tokens_array.tokens = NULL;
    }
    context->current = NULL;
    context->current_line = 0;
}


void runLexer(LexerContext* context)
{
    LEXER_ASSERT(context);

    while (true) {
        if (context->tokens_array.count == context->tokens_array.capacity) {
            expandTokensArray(context);
        }
        parseSpaces(context);

        if (context->current[0] == '\0') {
            parseEof(context);
            break;
        }

        switch (context->current[0]) {
            case '(': parseLeftParen(context);   break;
            case ')': parseRightParen(context);  break;
            case ';': parseSemicolon(context);   break;
            case '{': parseLeftBrace(context);   break;
            case '}': parseRightBrace(context);  break;
            case '+': parseOpAdd(context);       break;
            case '-': parseOpSub(context);       break;
            case '*': parseOpMul(context);       break;
            case '/': parseOpDiv(context);       break;
            case '=': parseOpAssign(context);    break;
            default:
                if (isdigit(context->current[0])) {
                    parseNumber(context);        break;
                } else if (isalpha(context->current[0]) || context->current[0] == '_') {
                    parseIdentifier(context);    break;
                } else {
                    parseError(context);         break;
                }
        }
    }
}


static void expandTokensArray(LexerContext* context)
{
    LEXER_ASSERT(context);

    Token* tokens = (Token*)calloc(context->tokens_array.capacity * 2, sizeof(Token));
    assert(tokens);

    context->tokens_array.tokens = tokens;
    context->tokens_array.capacity *= 2;
}


static void parseSpaces(LexerContext* context)
{
    LEXER_ASSERT(context);

    while (isspace(context->current[0])) {
        if (context->current[0] == '\n') {
            context->current_line++;
        }
        context->current++;
    }
}


static void parseEof(LexerContext* context)
{
    LEXER_ASSERT(context);

    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = TOKEN_EOF;
    token->start = context->current;
    token->length = 1;
    token->line = context->current_line;

    context->current++;
}


static void parseLeftParen(LexerContext* context)
{
    LEXER_ASSERT(context);

    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = TOKEN_LEFT_PAREN;
    token->start = context->current;
    token->length = 1;
    token->line = context->current_line;

    context->current++;
}


static void parseRightParen(LexerContext* context)
{
    LEXER_ASSERT(context);

    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = TOKEN_RIGHT_PAREN;
    token->start = context->current;
    token->length = 1;
    token->line = context->current_line;

    context->current++;
}


static void parseSemicolon(LexerContext* context)
{
    LEXER_ASSERT(context);

    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = TOKEN_SEMICOLON;
    token->start = context->current;
    token->length = 1;
    token->line = context->current_line;

    context->current++;
}


static void parseLeftBrace(LexerContext* context)
{
    LEXER_ASSERT(context);

    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = TOKEN_LEFT_BRACE;
    token->start = context->current;
    token->length = 1;
    token->line = context->current_line;

    context->current++;
}


static void parseRightBrace(LexerContext* context)
{
    LEXER_ASSERT(context);

    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = TOKEN_RIGHT_BRACE;
    token->start = context->current;
    token->length = 1;
    token->line = context->current_line;

    context->current++;
}


static void parseOpAdd(LexerContext* context)
{
    LEXER_ASSERT(context);

    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = TOKEN_OP_ADD;
    token->start = context->current;
    token->length = 1;
    token->line = context->current_line;

    context->current++;
}


static void parseOpSub(LexerContext* context)
{
    LEXER_ASSERT(context);

    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = TOKEN_OP_SUB;
    token->start = context->current;
    token->length = 1;
    token->line = context->current_line;

    context->current++;
}


static void parseOpMul(LexerContext* context)
{
    LEXER_ASSERT(context);

    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = TOKEN_OP_MUL;
    token->start = context->current;
    token->length = 1;
    token->line = context->current_line;

    context->current++;
}


static void parseOpDiv(LexerContext* context)
{
    LEXER_ASSERT(context);

    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = TOKEN_OP_DIV;
    token->start = context->current;
    token->length = 1;
    token->line = context->current_line;

    context->current++;
}


static void parseOpAssign(LexerContext* context)
{
    LEXER_ASSERT(context);

    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = TOKEN_OP_ASSIGN;
    token->start = context->current;
    token->length = 1;
    token->line = context->current_line;

    context->current++;
}


static void parseNumber(LexerContext* context)
{
    LEXER_ASSERT(context);

    const char* start = context->current;
    const char* end = context->current;
    while (isdigit(end[0])) {
        end++;
    }

    if (isalpha(end[0])) {
        start = end;
        while (isalnum(end[0]) || end[0] == '_') {
            end++;
        }

        size_t error_length = (size_t)(end - start);
        printf("length: %zu\n", error_length);
        reportLexerError(context, start, error_length,
            "Invalid number literal. Unexpected characters following a numeric value");

        context->current = end;
        return;
    }
    
    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = TOKEN_NUMBER;
    token->start = start;
    token->length = (size_t)(end - start);
    token->line = context->current_line;

    context->current = end;
}


static void parseIdentifier(LexerContext* context)
{
    LEXER_ASSERT(context);

    const char* end = context->current;
    while (isalpha(end[0]) || isdigit(end[0]) || end[0] == '_') {
        end++;
    }

    size_t length = (size_t)(end - context->current);
    bool is_keyword = false;
    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    for (size_t index = 0; index < KEYWORDS_LENGTH; index++) {
        if (KEYWORDS_TABLE[index].length == length &&
            strncmp(context->current, KEYWORDS_TABLE[index].text, length) == 0) {
            token->type = KEYWORDS_TABLE[index].type;
            token->start = context->current;
            token->length = length;
            token->line = context->current_line;
            is_keyword = true;
            break;
        }
    }

    if (!is_keyword) {
        token->type = TOKEN_IDENTIFIER;
        token->start = context->current;
        token->length = length;
        token->line = context->current_line;
    }

    context->current = end;
}


static void parseError(LexerContext* context)
{
    LEXER_ASSERT(context);

    const char* error_start = context->current;

    while (context->current[0] != '\0' && !isspace(context->current[0]) && 
        !isValidToken(context->current[0])) {
        context->current++;
    }

    size_t error_length = (size_t)(context->current - error_start);

    if (error_length == 0) {
        error_length = 1;
        context->current++;
    }

    reportLexerError(context, error_start, error_length,
        "Unknown token or invalid character sequence encountered");
}


static bool isValidToken(char token)
{
    const char* valid_tokens = "();{}+-*/=";

    if (isalnum(token) || token == '_') {
        return true;
    }

    for (size_t index = 0; index < strlen(valid_tokens); index++) {
        if (token == valid_tokens[index]) {
            return true;
        }
    }

    return false;
}