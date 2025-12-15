#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "lexer/lexer.h"
#include "lexer/lexer_utils.h"
#include "lexer/token.h"
#include "lexer/token_handlers.h"
#include "lexer/token_handlers.h"
#include "io/reader.h"


static void expandTokensArray(LexerContext* context);
static void parseSpaces(LexerContext* context);
static void parseEof(LexerContext* context);
static void parseKeyword(LexerContext* context, size_t keyword_table_idx);
static void parseNumber(LexerContext* context);
static void parseIdentifier(LexerContext* context);
static void parseError(LexerContext* context);
static bool isValidToken(char token);


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

        bool is_keyword = false;
        for (size_t index = 0; index < KEYWORD_TABLE_SIZE; index++) {
            if (strncmp(context->current, KEYWORD_TABLE[index].text, KEYWORD_TABLE[index].length) == 0) {
                parseKeyword(context, index);
                is_keyword = true;
                break;
            }
        }
        if (is_keyword) {
            continue;
        }

        if (isdigit(context->current[0])) {
            parseNumber(context);
            continue;
        } 
        
        if (isalpha(context->current[0]) || context->current[0] == '_' || !isspace(context->current[0])) {
            parseIdentifier(context);
            continue;
        }

        parseError(context);
        break;
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

    context->current += token->length;
}


static void parseKeyword(LexerContext* context, size_t keyword_table_idx)
{
    LEXER_ASSERT(context); assert(keyword_table_idx < KEYWORD_TABLE_SIZE);

    Token* token = &context->tokens_array.tokens[context->tokens_array.count++];

    token->type = KEYWORD_TABLE[keyword_table_idx].type;
    token->start = context->current;
    token->length = KEYWORD_TABLE[keyword_table_idx].length;
    token->line = context->current_line;

    context->current += token->length;
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

    for (size_t index = 0; index < KEYWORD_TABLE_SIZE; index++) {
        if (KEYWORD_TABLE[index].length == length &&
            strncmp(context->current, KEYWORD_TABLE[index].text, length) == 0) {

            token->type = KEYWORD_TABLE[index].type;
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